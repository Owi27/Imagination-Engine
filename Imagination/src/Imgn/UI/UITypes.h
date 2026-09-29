#pragma once
#include "Imgn/ImgnCore.hpp"

#include <cstdint>
#include <string>

namespace Imgn
{
	static constexpr uint32_t UIReferenceWidth = 1920;
	static constexpr uint32_t UIReferenceHeight = 1080;
	static constexpr uint32_t MaxUIQuads = 1024;
	static constexpr int32_t UISolidTexture = -1;
	static constexpr float UISceneBlitFlag = 1.f;

	enum class eUICanvasMode
	{
		ScreenSpace,
		WorldSpace
	};

	enum class eUIState
	{
		Normal,
		Hovered,
		Pressed,
		Disabled,
		Focused
	};

	enum class eUITextAlignment
	{
		Left,
		Center,
		Right
	};

	enum class eUIProperty
	{
		Opacity,
		PositionX,
		PositionY,
		SizeX,
		SizeY
	};

	enum class eUIWidgetType : uint32_t
	{
		Element = 0,
		Panel = 1,
		Image = 2,
		Text = 3,
		Button = 4,
		ProgressBar = 5,
		Slider = 6,
		Checkbox = 7,
		ScrollView = 8,
		HorizontalLayout = 9,
		VerticalLayout = 10,
		GridLayout = 11,
		OverlayLayout = 12
	};

	//gpu quad. 64 bytes, four float4s, matches UI.hlsl UIQuad.
	//extra: x texture index (negative = solid color), y scene-blit flag, z rotation radians, w unused
	struct UIQuadGPU
	{
		float rect[4];
		float uv[4];
		float color[4];
		float extra[4];
	};

	struct UIPC
	{
		float targetWidth;
		float targetHeight;
		uint32_t firstQuad;
		uint32_t pad;
	};

	struct UIRect
	{
		float x = 0.f, y = 0.f, width = 0.f, height = 0.f;

		float Right() const { return x + width; }
		float Bottom() const { return y + height; }

		bool Contains(float pX, float pY) const
		{
			return pX >= x && pY >= y && pX < Right() && pY < Bottom();
		}
	};

	struct UITransform
	{
		vec2 anchorMin = { 0.f, 0.f };
		vec2 anchorMax = { 0.f, 0.f };
		vec2 pivot = { 0.5f, 0.5f };
		vec2 position = { 0.f, 0.f };
		vec2 size = { 100.f, 100.f };
		vec2 minSize = { 0.f, 0.f };
		vec2 maxSize = { 0.f, 0.f };
		float rotation = 0.f;
	};

	struct UILayoutResult
	{
		UIRect rect;
		float clipMinX = 0.f, clipMinY = 0.f;
		float clipMaxX = 0.f, clipMaxY = 0.f;

		bool HasClip() const { return clipMaxX > clipMinX && clipMaxY > clipMinY; }
	};

	struct UIButtonStyle
	{
		vec4 normal = { 0.16f, 0.17f, 0.20f, 0.92f };
		vec4 hovered = { 0.24f, 0.28f, 0.34f, 0.96f };
		vec4 pressed = { 0.10f, 0.36f, 0.55f, 1.f };
		vec4 disabled = { 0.12f, 0.12f, 0.12f, 0.45f };
		vec4 focused = { 0.20f, 0.26f, 0.32f, 0.96f };
	};

	//flat child record the canvas component can round-trip through fstream
	struct UIWidgetRecord
	{
		eUIWidgetType type = eUIWidgetType::Panel;
		std::string name;
		std::string text;
		float anchorMin[2] = { 0.f, 0.f };
		float anchorMax[2] = { 0.f, 0.f };
		float pivot[2] = { 0.5f, 0.5f };
		float position[2] = { 0.f, 0.f };
		float size[2] = { 100.f, 100.f };
		float minSize[2] = { 0.f, 0.f };
		float maxSize[2] = { 0.f, 0.f };
		float rotation = 0.f;
		float color[4] = { 1.f, 1.f, 1.f, 1.f };
		float value = 0.f;
		uint8_t visible = 1;
		uint8_t enabled = 1;
	};
}
