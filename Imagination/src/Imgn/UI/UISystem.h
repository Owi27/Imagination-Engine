#pragma once
#include "Imgn/UI/UIAnimator.h"
#include "Imgn/UI/UICanvas.h"
#include "Imgn/UI/UIInput.h"
#include "Imgn/UI/UIRenderer.h"

namespace Imgn
{
	class RenderContext;
	class UIText;
	class UIPanel;

	class IMGN_API UISystem
	{
		static inline unique<UISystem> _instance;

		UIRenderer _renderer;
		UIInputSystem _input;
		UIAnimator _animator;
		std::vector<UICanvas*> _registered;
		std::vector<unique<UICanvas>> _owned;
		UIText* _fpsText = nullptr;
		UIPanel* _extraPanel = nullptr;
		bool _ready = false;
		bool _pollEngineInput = true;
		bool _testHUD = false;
		uint32_t _width = UIReferenceWidth;
		uint32_t _height = UIReferenceHeight;

		UISystem() = default;
		std::vector<UICanvas*> Gather() const;
		void PollEngineInput();

	public:
		//flip this to build the milestone HUD when the editor layer wakes up
		static constexpr bool TestHUDOnStartup = false;

		static UISystem& Get()
		{
			if (!_instance) _instance.reset(new UISystem());
			return *_instance;
		}

		bool Init(ImgnRenderer* pRenderer);
		void Shutdown();
		bool IsReady() const { return _ready; }

		void SetPollEngineInput(bool pPoll) { _pollEngineInput = pPoll; }
		void SetViewportPointer(bool pActive, float pX, float pY, bool pDown, bool pPressed, bool pReleased, float pScroll);

		void Resize(uint32_t pWidth, uint32_t pHeight);
		void Dream(float pDeltaTime);
		void ExecutePass(RenderContext& pContext, uint32_t pWidth, uint32_t pHeight);

		void RegisterCanvas(UICanvas* pCanvas);
		void UnregisterCanvas(UICanvas* pCanvas);
		UICanvas* CreateCanvas(const std::string& pName);

		void CreateTestHUD();
		void DestroyTestHUD();
		bool IsTestHUDActive() const { return _testHUD; }

		UIAnimator& GetAnimator() { return _animator; }
	};
}
