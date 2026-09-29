#pragma once
#include "Imgn/UI/UITypes.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Imgn
{
	class ImgnRenderer;

	struct UIGlyph
	{
		float u0 = 0.f, v0 = 0.f, u1 = 0.f, v1 = 0.f;
		bool present = false;
	};

	class IMGN_API UIFont
	{
		std::string _name;
		int32_t _textureIndex = UISolidTexture;
		UIGlyph _glyphs[128] = {};

	public:
		UIFont()
		{
			_name = "Default";
		}

		void SetName(const std::string& pName) { _name = pName; }
		const std::string& GetName() const { return _name; }
		void SetTextureIndex(int32_t pIndex) { _textureIndex = pIndex; }
		int32_t GetTextureIndex() const { return _textureIndex; }
		void SetGlyph(uint32_t pCode, const UIGlyph& pGlyph)
		{
			if (pCode > 127) return;
			_glyphs[pCode] = pGlyph;
		}

		bool TryGetGlyph(uint32_t pCode, UIGlyph& pGlyph) const
		{
			if (pCode > 127) return false;
			if (!_glyphs[pCode].present) return false;
			pGlyph = _glyphs[pCode];
			return true;
		}
	};

	//owns the ascii atlas. RasterizeBuiltin is the seam a FreeType backend replaces.
	class IMGN_API UIFontManager
	{
		static inline unique<UIFontManager> _instance;

		std::vector<uint8_t> _pixels;
		uint32_t _width = 0, _height = 0;
		unique<UIFont> _default;
		int32_t _textureIndex = UISolidTexture;

		UIFontManager() = default;

	public:
		static UIFontManager& Get()
		{
			if (!_instance) _instance.reset(new UIFontManager());
			return *_instance;
		}

		//builds an 8x8 ascii atlas from the inlined bitmap. no FreeType.
		void RasterizeBuiltin();
		bool Upload(ImgnRenderer* pRenderer);

		uint32_t GetWidth() const { return _width; }
		uint32_t GetHeight() const { return _height; }
		const std::vector<uint8_t>& GetPixels() const { return _pixels; }
		int32_t GetTextureIndex() const { return _textureIndex; }
		UIFont* Find(const std::string& pName);
	};
}
