#include "pch.hpp"
#include "Imgn/UI/UICanvasComponent.h"

#include "Imgn/UI/UIControls.h"
#include "Imgn/UI/UILayout.h"
#include "Imgn/UI/UISystem.h"

namespace Imgn
{
	void UICanvasComponent::WriteString(std::fstream& pStream, const std::string& pValue) const
	{
		const uint32_t length = static_cast<uint32_t>(pValue.size());
		pStream.write(reinterpret_cast<const char*>(&length), sizeof(length));
		if (length > 0) pStream.write(pValue.data(), length);
	}

	std::string UICanvasComponent::ReadString(std::fstream& pStream) const
	{
		uint32_t length = 0;
		pStream.read(reinterpret_cast<char*>(&length), sizeof(length));
		if (!pStream || length > 1u << 20)
		{
			IMGN_CORE_ERROR("UI canvas string length {} is not readable.", length);
			return {};
		}

		std::string value(length, '\0');
		if (length > 0) pStream.read(value.data(), length);
		return value;
	}

	void UICanvasComponent::OnDestroy()
	{
		if (_canvas) UISystem::Get().UnregisterCanvas(_canvas.get());
	}

	void UICanvasComponent::Dream(float pDeltaTime)
	{
		(void)pDeltaTime;
		if (_canvas) UISystem::Get().RegisterCanvas(_canvas.get());
	}

	void UICanvasComponent::RebuildChildren()
	{
		if (!_canvas) return;

		//direct children only. nested control labels are rebuilt by the control constructor.
		unique<UICanvas> fresh = Unique<UICanvas>();
		fresh->SetName(_canvas->GetName());
		fresh->SetReferenceResolution(_canvas->GetReferenceWidth(), _canvas->GetReferenceHeight());
		fresh->SetMode(_canvas->GetMode());
		UISystem::Get().UnregisterCanvas(_canvas.get());
		_canvas = std::move(fresh);

		for (const UIWidgetRecord& record : _children)
		{
			UIElement* element = nullptr;
			switch (record.type)
			{
				case eUIWidgetType::Panel: element = _canvas->AddChild<UIPanel>(); break;
				case eUIWidgetType::Image: element = _canvas->AddChild<UIImage>(); break;
				case eUIWidgetType::Text: element = _canvas->AddChild<UIText>(); break;
				case eUIWidgetType::Button: element = _canvas->AddChild<UIButton>(); break;
				case eUIWidgetType::ProgressBar: element = _canvas->AddChild<UIProgressBar>(); break;
				case eUIWidgetType::Slider: element = _canvas->AddChild<UISlider>(); break;
				case eUIWidgetType::Checkbox: element = _canvas->AddChild<UICheckbox>(); break;
				case eUIWidgetType::ScrollView: element = _canvas->AddChild<UIScrollView>(); break;
				case eUIWidgetType::HorizontalLayout: element = _canvas->AddChild<UIHorizontalLayout>(); break;
				case eUIWidgetType::VerticalLayout: element = _canvas->AddChild<UIVerticalLayout>(); break;
				case eUIWidgetType::GridLayout: element = _canvas->AddChild<UIGridLayout>(); break;
				case eUIWidgetType::OverlayLayout: element = _canvas->AddChild<UIOverlayLayout>(); break;
				case eUIWidgetType::Element: element = _canvas->AddChild<UIElement>(); break;
			}

			if (!element) continue;

			element->SetName(record.name);
			element->SetVisible(record.visible != 0);
			element->SetEnabled(record.enabled != 0);
			UITransform& transform = element->GetTransform();
			transform.anchorMin = { record.anchorMin[0], record.anchorMin[1] };
			transform.anchorMax = { record.anchorMax[0], record.anchorMax[1] };
			transform.pivot = { record.pivot[0], record.pivot[1] };
			transform.position = { record.position[0], record.position[1] };
			transform.size = { record.size[0], record.size[1] };
			transform.minSize = { record.minSize[0], record.minSize[1] };
			transform.maxSize = { record.maxSize[0], record.maxSize[1] };
			transform.rotation = record.rotation;

			if (UIPanel* panel = dynamic_cast<UIPanel*>(element))
				panel->SetColor({ record.color[0], record.color[1], record.color[2], record.color[3] });
			if (UIText* text = dynamic_cast<UIText*>(element))
			{
				text->SetText(record.text);
				text->SetFontSize(record.value > 0.f ? record.value : 32.f);
				text->SetColor({ record.color[0], record.color[1], record.color[2], record.color[3] });
			}
			if (UIButton* button = dynamic_cast<UIButton*>(element)) button->SetText(record.text);
			if (UIProgressBar* bar = dynamic_cast<UIProgressBar*>(element)) bar->SetValue(record.value);
			if (UISlider* slider = dynamic_cast<UISlider*>(element)) slider->SetValue(record.value);
			if (UICheckbox* box = dynamic_cast<UICheckbox*>(element))
			{
				box->SetChecked(record.value >= 0.5f);
				box->SetText(record.text);
			}
			if (UIImage* image = dynamic_cast<UIImage*>(element))
			{
				image->SetColor({ record.color[0], record.color[1], record.color[2], record.color[3] });
				image->SetTextureIndex(static_cast<int32_t>(record.value));
			}
		}
	}

	void UICanvasComponent::Serialize(std::fstream& pStream)
	{
		pStream.write(reinterpret_cast<const char*>(&TypeID), sizeof(ID));

		_children.clear();
		if (_canvas)
		{
			for (const unique<UIElement>& child : _canvas->GetChildren())
			{
				if (!child) continue;
				_children.push_back(child->ToRecord());
			}
		}

		const uint32_t version = 1;
		pStream.write(reinterpret_cast<const char*>(&version), sizeof(version));

		std::string name = _canvas ? _canvas->GetName() : std::string("Canvas");
		WriteString(pStream, name);

		uint32_t referenceWidth = _canvas ? _canvas->GetReferenceWidth() : UIReferenceWidth;
		uint32_t referenceHeight = _canvas ? _canvas->GetReferenceHeight() : UIReferenceHeight;
		uint32_t mode = static_cast<uint32_t>(_canvas ? _canvas->GetMode() : eUICanvasMode::ScreenSpace);
		pStream.write(reinterpret_cast<const char*>(&referenceWidth), sizeof(referenceWidth));
		pStream.write(reinterpret_cast<const char*>(&referenceHeight), sizeof(referenceHeight));
		pStream.write(reinterpret_cast<const char*>(&mode), sizeof(mode));

		uint32_t count = static_cast<uint32_t>(_children.size());
		pStream.write(reinterpret_cast<const char*>(&count), sizeof(count));

		for (const UIWidgetRecord& record : _children)
		{
			uint32_t type = static_cast<uint32_t>(record.type);
			pStream.write(reinterpret_cast<const char*>(&type), sizeof(type));
			WriteString(pStream, record.name);
			WriteString(pStream, record.text);
			pStream.write(reinterpret_cast<const char*>(record.anchorMin), sizeof(record.anchorMin));
			pStream.write(reinterpret_cast<const char*>(record.anchorMax), sizeof(record.anchorMax));
			pStream.write(reinterpret_cast<const char*>(record.pivot), sizeof(record.pivot));
			pStream.write(reinterpret_cast<const char*>(record.position), sizeof(record.position));
			pStream.write(reinterpret_cast<const char*>(record.size), sizeof(record.size));
			pStream.write(reinterpret_cast<const char*>(record.minSize), sizeof(record.minSize));
			pStream.write(reinterpret_cast<const char*>(record.maxSize), sizeof(record.maxSize));
			pStream.write(reinterpret_cast<const char*>(&record.rotation), sizeof(record.rotation));
			pStream.write(reinterpret_cast<const char*>(record.color), sizeof(record.color));
			pStream.write(reinterpret_cast<const char*>(&record.value), sizeof(record.value));
			pStream.write(reinterpret_cast<const char*>(&record.visible), sizeof(record.visible));
			pStream.write(reinterpret_cast<const char*>(&record.enabled), sizeof(record.enabled));
		}
	}

	void UICanvasComponent::Deserialize(std::fstream& pStream)
	{
		uint32_t version = 0;
		pStream.read(reinterpret_cast<char*>(&version), sizeof(version));
		if (version != 1)
		{
			IMGN_CORE_ERROR("UI canvas serialize version {} is not supported.", version);
			return;
		}

		if (!_canvas) _canvas = Unique<UICanvas>();
		_canvas->SetName(ReadString(pStream));

		uint32_t referenceWidth = UIReferenceWidth;
		uint32_t referenceHeight = UIReferenceHeight;
		uint32_t mode = 0;
		pStream.read(reinterpret_cast<char*>(&referenceWidth), sizeof(referenceWidth));
		pStream.read(reinterpret_cast<char*>(&referenceHeight), sizeof(referenceHeight));
		pStream.read(reinterpret_cast<char*>(&mode), sizeof(mode));
		_canvas->SetReferenceResolution(referenceWidth, referenceHeight);
		_canvas->SetMode(static_cast<eUICanvasMode>(mode));

		uint32_t count = 0;
		pStream.read(reinterpret_cast<char*>(&count), sizeof(count));
		if (count > 4096)
		{
			IMGN_CORE_ERROR("UI canvas child count {} is not believable.", count);
			return;
		}

		_children.clear();
		_children.reserve(count);
		for (uint32_t i = 0; i < count; i++)
		{
			UIWidgetRecord record;
			uint32_t type = 0;
			pStream.read(reinterpret_cast<char*>(&type), sizeof(type));
			record.type = static_cast<eUIWidgetType>(type);
			record.name = ReadString(pStream);
			record.text = ReadString(pStream);
			pStream.read(reinterpret_cast<char*>(record.anchorMin), sizeof(record.anchorMin));
			pStream.read(reinterpret_cast<char*>(record.anchorMax), sizeof(record.anchorMax));
			pStream.read(reinterpret_cast<char*>(record.pivot), sizeof(record.pivot));
			pStream.read(reinterpret_cast<char*>(record.position), sizeof(record.position));
			pStream.read(reinterpret_cast<char*>(record.size), sizeof(record.size));
			pStream.read(reinterpret_cast<char*>(record.minSize), sizeof(record.minSize));
			pStream.read(reinterpret_cast<char*>(record.maxSize), sizeof(record.maxSize));
			pStream.read(reinterpret_cast<char*>(&record.rotation), sizeof(record.rotation));
			pStream.read(reinterpret_cast<char*>(record.color), sizeof(record.color));
			pStream.read(reinterpret_cast<char*>(&record.value), sizeof(record.value));
			pStream.read(reinterpret_cast<char*>(&record.visible), sizeof(record.visible));
			pStream.read(reinterpret_cast<char*>(&record.enabled), sizeof(record.enabled));
			_children.push_back(record);
		}

		RebuildChildren();
	}
}
