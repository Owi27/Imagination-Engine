#pragma once
#include "Imgn/UI/UITypes.h"

#include <vector>

namespace Imgn
{
	class UIElement;

	class IMGN_API UIAnimator
	{
		struct Tween
		{
			UIElement* element = nullptr;
			eUIProperty property = eUIProperty::Opacity;
			float from = 0.f;
			float to = 0.f;
			float duration = 0.f;
			float elapsed = 0.f;
		};

		std::vector<Tween> _tweens;

		static float Read(UIElement* pElement, eUIProperty pProperty);
		static void Write(UIElement* pElement, eUIProperty pProperty, float pValue);

	public:
		void Animate(UIElement* pElement, eUIProperty pProperty, float pFrom, float pTo, float pSeconds);
		void Tick(float pDeltaTime);
		void Clear() { _tweens.clear(); }
	};
}
