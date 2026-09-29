#include "pch.hpp"
#include "Imgn/UI/UIAnimator.h"

#include "Imgn/UI/UIElement.h"

namespace Imgn
{
	float UIAnimator::Read(UIElement* pElement, eUIProperty pProperty)
	{
		if (!pElement) return 0.f;

		switch (pProperty)
		{
			case eUIProperty::Opacity: return pElement->GetOpacity();
			case eUIProperty::PositionX: return pElement->GetTransform().position[0];
			case eUIProperty::PositionY: return pElement->GetTransform().position[1];
			case eUIProperty::SizeX: return pElement->GetTransform().size[0];
			case eUIProperty::SizeY: return pElement->GetTransform().size[1];
		}

		return 0.f;
	}

	void UIAnimator::Write(UIElement* pElement, eUIProperty pProperty, float pValue)
	{
		if (!pElement) return;

		switch (pProperty)
		{
			case eUIProperty::Opacity: pElement->SetOpacity(pValue); break;
			case eUIProperty::PositionX: pElement->GetTransform().position[0] = pValue; break;
			case eUIProperty::PositionY: pElement->GetTransform().position[1] = pValue; break;
			case eUIProperty::SizeX: pElement->GetTransform().size[0] = pValue; break;
			case eUIProperty::SizeY: pElement->GetTransform().size[1] = pValue; break;
		}
	}

	void UIAnimator::Animate(UIElement* pElement, eUIProperty pProperty, float pFrom, float pTo, float pSeconds)
	{
		if (!pElement) return;
		if (pSeconds < 0.f) pSeconds = 0.f;

		Write(pElement, pProperty, pFrom);

		Tween tween;
		tween.element = pElement;
		tween.property = pProperty;
		tween.from = pFrom;
		tween.to = pTo;
		tween.duration = pSeconds;
		tween.elapsed = 0.f;
		_tweens.push_back(tween);
	}

	void UIAnimator::Tick(float pDeltaTime)
	{
		if (pDeltaTime < 0.f) pDeltaTime = 0.f;

		for (uint32_t i = 0; i < _tweens.size();)
		{
			Tween& tween = _tweens[i];
			if (!tween.element)
			{
				_tweens.erase(_tweens.begin() + static_cast<std::ptrdiff_t>(i));
				continue;
			}

			tween.elapsed += pDeltaTime;
			float t = tween.duration <= 0.f ? 1.f : tween.elapsed / tween.duration;
			if (t > 1.f) t = 1.f;
			Write(tween.element, tween.property, tween.from + (tween.to - tween.from) * t);

			if (t >= 1.f)
			{
				_tweens.erase(_tweens.begin() + static_cast<std::ptrdiff_t>(i));
				continue;
			}

			i++;
		}
	}
}
