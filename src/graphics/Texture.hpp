#ifndef BLANK_GRAPHICS_TEXTURE_HPP_
#define BLANK_GRAPHICS_TEXTURE_HPP_

#include "TextureBase.hpp"

#include <GL/glew.h>

struct SDL_Surface;


namespace blank {

struct Format;

class Texture
: public TextureBase<GL_TEXTURE_2D> {

public:
	Texture();
	~Texture();

	Texture(Texture &&) noexcept;
	Texture &operator =(Texture &&) noexcept;

	Texture(const Texture &) = delete;
	Texture &operator =(const Texture &) = delete;

public:
	GLsizei Width() const noexcept { return width; }
	GLsizei Height() const noexcept { return height; }

	void Data(const SDL_Surface &, bool pad2 = true) noexcept;
	void Data(GLsizei w, GLsizei h, const Format &, GLvoid *data) noexcept;

	static void UnpackAlignment(GLint) noexcept;
	static int UnpackAlignmentFromPitch(int) noexcept;
	static void UnpackRowLength(GLint) noexcept;

private:
	GLsizei width, height;

};

}

#endif
