#ifndef BLANK_GRAPHICS_FONT_HPP_
#define BLANK_GRAPHICS_FONT_HPP_

#include <SDL_ttf.h>
#include <glm/glm.hpp>


namespace blank {

class Texture;

class Font {

public:
	Font(const char *src, int size, long index = 0);
	~Font();

	Font(Font &&) noexcept;
	Font &operator =(Font &&) noexcept;

	Font(const Font &) = delete;
	Font &operator =(const Font &) = delete;

public:
	bool Kerning() const noexcept;
	void Kerning(bool) noexcept;

	int Height() const noexcept;
	int Ascent() const noexcept;
	int Descent() const noexcept;
	int LineSkip() const noexcept;

	bool HasGlyph(Uint16) const noexcept;

	glm::tvec2<int> TextSize(const char *) const;

	Texture Render(const char *, SDL_Color) const;

private:
	TTF_Font *handle;

};

}

#endif
