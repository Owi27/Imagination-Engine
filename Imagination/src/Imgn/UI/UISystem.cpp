#include "pch.hpp"
#include "Imgn/UI/UISystem.h"

#include "Imgn/ImgnInput.h"
#include "Imgn/ImgnRenderContext.h"
#include "Imgn/UI/UIControls.h"
#include "Imgn/UI/UIFont.h"
#include "Imgn/UI/UILayout.h"

#include "ImGui/imgui.h"

namespace Imgn
{
	std::vector<UICanvas*> UISystem::Gather() const
	{
		std::vector<UICanvas*> canvases;
		canvases.reserve(_registered.size() + _owned.size());

		for (UICanvas* canvas : _registered)
		{
			if (canvas) canvases.push_back(canvas);
		}

		for (const unique<UICanvas>& canvas : _owned)
		{
			if (canvas) canvases.push_back(canvas.get());
		}

		return canvases;
	}

	void UISystem::PollEngineInput()
	{
		static bool wasDown = false;

		UIPointerFrame frame;
		//editor viewports must not use this path. ImGui sets WantCaptureMouse for the scene image too.
		if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse)
		{
			frame.active = false;
			wasDown = false;
			_input.SetFrame(frame);
			return;
		}

		const std::pair<float, float> position = Input::GetMousePosition();
		const bool down = Input::IsMouseButtonPressed(IMGN_MOUSE_BUTTON_LEFT);
		frame.active = true;
		frame.x = position.first;
		frame.y = position.second;
		frame.down = down;
		frame.pressed = down && !wasDown;
		frame.released = !down && wasDown;
		frame.scroll = 0.f;
		wasDown = down;
		_input.SetFrame(frame);
	}

	bool UISystem::Init(ImgnRenderer* pRenderer)
	{
		if (_ready) return true;
		if (!pRenderer)
		{
			IMGN_CORE_ERROR("UISystem::Init received a null renderer.");
			return false;
		}

		UIFontManager::Get().RasterizeBuiltin();
		if (!UIFontManager::Get().Upload(pRenderer)) return false;
		if (!_renderer.Init(pRenderer)) return false;

		_ready = true;
		IMGN_CORE_INFO("Retained UI system ready. Test HUD startup flag is {}.", TestHUDOnStartup ? "on" : "off");
		return true;
	}

	void UISystem::Shutdown()
	{
		DestroyTestHUD();
		_registered.clear();
		_ready = false;
	}

	void UISystem::SetViewportPointer(bool pActive, float pX, float pY, bool pDown, bool pPressed, bool pReleased, float pScroll)
	{
		UIPointerFrame frame;
		frame.active = pActive;
		frame.x = pX;
		frame.y = pY;
		frame.down = pDown;
		frame.pressed = pPressed;
		frame.released = pReleased;
		frame.scroll = pScroll;
		_input.SetFrame(frame);
	}

	void UISystem::Resize(uint32_t pWidth, uint32_t pHeight)
	{
		if (pWidth == 0 || pHeight == 0) return;
		_width = pWidth;
		_height = pHeight;

		for (UICanvas* canvas : Gather())
			canvas->Resize(pWidth, pHeight);
	}

	void UISystem::Dream(float pDeltaTime)
	{
		if (!_ready) return;
		if (_pollEngineInput) PollEngineInput();

		const std::vector<UICanvas*> canvases = Gather();
		_input.Apply(canvases);
		_animator.Tick(pDeltaTime);

		for (UICanvas* canvas : canvases)
		{
			if (!canvas || canvas->GetMode() != eUICanvasMode::ScreenSpace) continue;
			canvas->Dream(pDeltaTime);
			UILayoutSystem::CalculateLayout(canvas);
		}

		if (_fpsText && pDeltaTime > 0.f)
			_fpsText->SetText(std::format("{:.0f} FPS", 1.f / pDeltaTime));
	}

	void UISystem::ExecutePass(RenderContext& pContext, uint32_t pWidth, uint32_t pHeight)
	{
		if (!_ready) return;

		_renderer.Clear();
		for (UICanvas* canvas : Gather())
		{
			if (!canvas || !canvas->IsVisible()) continue;
			if (canvas->GetMode() != eUICanvasMode::ScreenSpace) continue;
			canvas->CollectQuads(_renderer.Quads(), 1.f);
		}

		_renderer.Render(pContext, pWidth, pHeight);
	}

	void UISystem::RegisterCanvas(UICanvas* pCanvas)
	{
		if (!pCanvas) return;
		for (UICanvas* canvas : _registered)
		{
			if (canvas == pCanvas) return;
		}
		_registered.push_back(pCanvas);
		pCanvas->Resize(_width, _height);
	}

	void UISystem::UnregisterCanvas(UICanvas* pCanvas)
	{
		for (uint32_t i = 0; i < _registered.size(); i++)
		{
			if (_registered[i] != pCanvas) continue;
			_registered.erase(_registered.begin() + static_cast<std::ptrdiff_t>(i));
			return;
		}
	}

	UICanvas* UISystem::CreateCanvas(const std::string& pName)
	{
		unique<UICanvas> canvas = Unique<UICanvas>();
		canvas->SetName(pName);
		canvas->Resize(_width, _height);
		UICanvas* pointer = canvas.get();
		_owned.push_back(std::move(canvas));
		return pointer;
	}

	void UISystem::DestroyTestHUD()
	{
		_fpsText = nullptr;
		_extraPanel = nullptr;
		_owned.clear();
		_animator.Clear();
		_testHUD = false;
	}

	void UISystem::CreateTestHUD()
	{
		if (!_ready)
		{
			IMGN_CORE_ERROR("CreateTestHUD called before UISystem::Init.");
			return;
		}

		DestroyTestHUD();

		UICanvas* canvas = CreateCanvas("TestHUD");
		canvas->Resize(_width, _height);

		_fpsText = canvas->AddChild<UIText>();
		_fpsText->SetName("FPS");
		_fpsText->SetText("FPS");
		_fpsText->SetFontSize(28.f);
		_fpsText->SetAlignment(eUITextAlignment::Right);
		_fpsText->SetColor({ 1.f, 1.f, 1.f, 1.f });
		_fpsText->GetTransform().anchorMin = { 1.f, 0.f };
		_fpsText->GetTransform().anchorMax = { 1.f, 0.f };
		_fpsText->GetTransform().pivot = { 1.f, 0.f };
		_fpsText->GetTransform().position = { -24.f, 16.f };
		_fpsText->GetTransform().size = { 280.f, 40.f };

		UIImage* crossX = canvas->AddChild<UIImage>();
		crossX->SetName("CrosshairX");
		crossX->SetColor({ 1.f, 1.f, 1.f, 0.9f });
		crossX->GetTransform().anchorMin = { 0.5f, 0.5f };
		crossX->GetTransform().anchorMax = { 0.5f, 0.5f };
		crossX->GetTransform().pivot = { 0.5f, 0.5f };
		crossX->GetTransform().size = { 18.f, 2.f };

		UIImage* crossY = canvas->AddChild<UIImage>();
		crossY->SetName("CrosshairY");
		crossY->SetColor({ 1.f, 1.f, 1.f, 0.9f });
		crossY->GetTransform().anchorMin = { 0.5f, 0.5f };
		crossY->GetTransform().anchorMax = { 0.5f, 0.5f };
		crossY->GetTransform().pivot = { 0.5f, 0.5f };
		crossY->GetTransform().size = { 2.f, 18.f };

		UIPanel* health = canvas->AddChild<UIPanel>();
		health->SetName("Health");
		health->SetColor({ 0.06f, 0.07f, 0.09f, 0.82f });
		health->GetTransform().anchorMin = { 0.f, 1.f };
		health->GetTransform().anchorMax = { 0.f, 1.f };
		health->GetTransform().pivot = { 0.f, 1.f };
		health->GetTransform().position = { 28.f, -28.f };
		health->GetTransform().size = { 340.f, 168.f };

		UIVerticalLayout* column = health->AddChild<UIVerticalLayout>();
		column->SetName("HealthColumn");
		column->SetSpacing(8.f);
		column->SetPadding(16.f);
		column->GetTransform().anchorMin = { 0.f, 0.f };
		column->GetTransform().anchorMax = { 1.f, 1.f };
		column->GetTransform().pivot = { 0.5f, 0.5f };
		column->GetTransform().size = { 0.f, 0.f };

		UIProgressBar* bar = column->AddChild<UIProgressBar>();
		bar->SetName("HealthBar");
		bar->SetValue(0.75f);
		bar->GetTransform().size = { 300.f, 22.f };

		UIText* healthText = column->AddChild<UIText>();
		healthText->SetName("HealthText");
		healthText->SetText("75 / 100");
		healthText->SetFontSize(24.f);
		healthText->SetAlignment(eUITextAlignment::Left);
		healthText->GetTransform().size = { 300.f, 28.f };

		UIButton* button = column->AddChild<UIButton>();
		button->SetName("Toggle");
		button->SetText("Toggle");
		button->GetTransform().size = { 160.f, 36.f };

		_extraPanel = canvas->AddChild<UIPanel>();
		_extraPanel->SetName("Extra");
		_extraPanel->SetVisible(false);
		_extraPanel->SetColor({ 0.10f, 0.11f, 0.14f, 0.9f });
		_extraPanel->GetTransform().anchorMin = { 0.f, 1.f };
		_extraPanel->GetTransform().anchorMax = { 0.f, 1.f };
		_extraPanel->GetTransform().pivot = { 0.f, 1.f };
		_extraPanel->GetTransform().position = { 384.f, -28.f };
		_extraPanel->GetTransform().size = { 280.f, 120.f };

		UISlider* slider = _extraPanel->AddChild<UISlider>();
		slider->SetName("Volume");
		slider->SetValue(0.4f);
		slider->GetTransform().anchorMin = { 0.f, 0.f };
		slider->GetTransform().anchorMax = { 1.f, 0.f };
		slider->GetTransform().pivot = { 0.f, 0.f };
		slider->GetTransform().position = { 16.f, 16.f };
		slider->GetTransform().size = { -32.f, 28.f };

		UICheckbox* checkbox = _extraPanel->AddChild<UICheckbox>();
		checkbox->SetName("Mute");
		checkbox->SetText("Mute");
		checkbox->GetTransform().anchorMin = { 0.f, 0.f };
		checkbox->GetTransform().anchorMax = { 1.f, 0.f };
		checkbox->GetTransform().pivot = { 0.f, 0.f };
		checkbox->GetTransform().position = { 16.f, 56.f };
		checkbox->GetTransform().size = { -32.f, 32.f };

		button->OnClick = [this]()
		{
			if (!_extraPanel) return;
			_extraPanel->SetVisible(!_extraPanel->IsVisible());
			IMGN_CORE_INFO("UI test button toggled extra panel to {}.", _extraPanel->IsVisible() ? "visible" : "hidden");
		};

		_animator.Animate(health, eUIProperty::Opacity, 0.f, 1.f, 0.35f);
		_testHUD = true;
		IMGN_CORE_INFO("Test HUD created. Turn it off from View > Test HUD.");
	}
}
