#include "pch.hpp"
#include "Imgn/UI/UICanvas.h"

#include <algorithm>

namespace Imgn
{
	void UICanvas::Resize(uint32_t pWidth, uint32_t pHeight)
	{
		if (_mode == eUICanvasMode::WorldSpace)
		{
			IMGN_CORE_ERROR("World-space UI canvas '{}' is not implemented.", GetName());
			IMGN_CORE_ASSERT(false, "World-space UI canvas is not implemented.");
			return;
		}

		if (pWidth == 0 || pHeight == 0) return;

		_pixelWidth = pWidth;
		_pixelHeight = pHeight;

		const float scaleX = static_cast<float>(pWidth) / static_cast<float>(_referenceWidth);
		const float scaleY = static_cast<float>(pHeight) / static_cast<float>(_referenceHeight);
		_scale = std::min(scaleX, scaleY);

		GetTransform().size = { static_cast<float>(pWidth), static_cast<float>(pHeight) };
	}

	UIRect UICanvas::ContentRect() const
	{
		UIRect rect;
		rect.x = 0.f;
		rect.y = 0.f;
		rect.width = static_cast<float>(_pixelWidth);
		rect.height = static_cast<float>(_pixelHeight);
		return rect;
	}
}
