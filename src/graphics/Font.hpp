#ifndef BLANK_GRAPHICS_FONT_HPP_
#define BLANK_GRAPHICS_FONT_HPP_

#include "glm.hpp"

#include <SDL_ttf.h>


namespace blank {

class Texture;

class Font {

public:
	enum FontStyle {
		STYLE_NORMAL = TTF_STYLE_NORMAL,
		STYLE_BOLD = TTF_STYLE_BOLD,
		STYLE_ITALIC = TTF_STYLE_ITALIC,
		STYLE_UNDERLINE = TTF_STYLE_UNDERLINE,
		STYLE_STRIKE = TTF_STYLE_STRIKETHROUGH,
	};
	enum FontHinting {
		HINT_NORMAL = TTF_HINTING_NORMAL,
		HINT_LIGHT = TTF_HINTING_LIGHT,
		HINT_MONO = TTF_HINTING_MONO,
		HINT_NONE = TTF_HINTING_NONE,
	};

public:
	Font(const char *src, int size, long index = 0);
	~Font();

	Font(Font &&) noexcept;
	Font &operator =(Font &&) noexcept;

	Font(const Font &) = delete;
	Font &operator =(const Font &) = delete;

public:
	int Style() const noexcept;
	void Style(int) const noexcept;
	int Outline() const noexcept;
	void Outline(int) noexcept;

	int Hinting() const noexcept;
	void Hinting(int) const noexcept;
	bool Kerning() const noexcept;
	void Kerning(bool) noexcept;

	int Height() const noexcept;
	int Ascent() const noexcept;
	int Descent() const noexcept;
	int LineSkip() const noexcept;

	const char *FamilyName() const noexcept;
	const char *StyleName() const noexcept;

	bool HasGlyph(Uint16) const noexcept;

	glm::ivec2 TextSize(const char *) const;

	Texture Render(const char *) const;
	void Render(const char *, Texture &) const;

private:
	TTF_Font *handle;

};

}

#endif
