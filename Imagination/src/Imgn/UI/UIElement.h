#pragma once
#include "Imgn/UI/UITypes.h"

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace Imgn
{
	struct UIDrawQuad
	{
		UIQuadGPU gpu;
		float clipMinX = 0.f, clipMinY = 0.f, clipMaxX = 0.f, clipMaxY = 0.f;
	};

	class IMGN_API UIElement
	{
		void SetParent(UIElement* pParent) { _parent = pParent; }

		std::string _name = "Element";
		bool _visible = true;
		bool _enabled = true;
		bool _clipChildren = false;
		bool _receivesPointer = true;
		float _opacity = 1.f;
		float _canvasScale = 1.f;
		UIElement* _parent = nullptr;
		std::vector<unique<UIElement>> _children;
		UITransform _transform;
		UILayoutResult _layout;

	protected:
		void SetReceivesPointer(bool pReceives) { _receivesPointer = pReceives; }
		float GetCanvasScale() const { return _canvasScale; }

		virtual void ArrangeChildren() {}
		virtual void OnUpdate(float pDeltaTime) {}
		virtual void Emit(std::vector<UIDrawQuad>& pQuads, float pOpacity) {}
		virtual void FillRecord(UIWidgetRecord& pRecord) const {}

	public:
		UIElement() = default;
		virtual ~UIElement() = default;

		UIElement(const UIElement&) = delete;
		UIElement& operator=(const UIElement&) = delete;

		virtual eUIWidgetType GetWidgetType() const { return eUIWidgetType::Element; }

		void SetName(const std::string& pName) { _name = pName; }
		const std::string& GetName() const { return _name; }

		void SetVisible(bool pVisible) { _visible = pVisible; }
		bool IsVisible() const { return _visible; }

		void SetEnabled(bool pEnabled) { _enabled = pEnabled; }
		bool IsEnabled() const { return _enabled; }

		void SetClipChildren(bool pClip) { _clipChildren = pClip; }
		bool GetClipChildren() const { return _clipChildren; }

		void SetOpacity(float pOpacity) { _opacity = pOpacity; }
		float GetOpacity() const { return _opacity; }

		UITransform& GetTransform() { return _transform; }
		const UITransform& GetTransform() const { return _transform; }

		const UILayoutResult& GetLayout() const { return _layout; }
		UIElement* GetParent() const { return _parent; }
		const std::vector<unique<UIElement>>& GetChildren() const { return _children; }

		template<typename T, typename... Args>
		T* AddChild(Args&&... pArgs)
		{
			static_assert(std::is_base_of<UIElement, T>::value, "T must derive from UIElement");
			unique<T> child = Unique<T>(std::forward<Args>(pArgs)...);
			T* pointer = child.get();
			pointer->SetParent(this);
			_children.push_back(std::move(child));
			return pointer;
		}

		void Dream(float pDeltaTime);
		virtual void Update(float pDeltaTime) { OnUpdate(pDeltaTime); }

		virtual void OnPointerEnter() {}
		virtual void OnPointerExit() {}
		virtual void OnPointerDown(float pX, float pY) {}
		virtual void OnPointerUp(float pX, float pY) {}
		virtual void OnClick() {}
		virtual void OnDrag(float pX, float pY, float pDeltaX, float pDeltaY) {}
		virtual void OnScroll(float pDelta) {}

		bool IsPointInside(float pX, float pY) const;
		UIElement* HitTest(float pX, float pY);

		void Layout(const UIRect& pParent, float pClipMinX, float pClipMinY, float pClipMaxX, float pClipMaxY, float pScale);
		void LayoutAsRoot(const UIRect& pRect, float pScale);
		void CollectQuads(std::vector<UIDrawQuad>& pQuads, float pParentOpacity);

		UIWidgetRecord ToRecord() const;
	};
}
