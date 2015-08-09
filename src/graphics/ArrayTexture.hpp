#ifndef BLANK_GRAPHICS_ARRAYTEXTURE_HPP_
#define BLANK_GRAPHICS_ARRAYTEXTURE_HPP_

#include "Format.hpp"

#include <GL/glew.h>

struct SDL_Surface;


namespace blank {

class ArrayTexture {

public:
	ArrayTexture();
	~ArrayTexture();

	ArrayTexture(ArrayTexture &&) noexcept;
	ArrayTexture &operator =(ArrayTexture &&) noexcept;

	ArrayTexture(const ArrayTexture &) = delete;
	ArrayTexture &operator =(const ArrayTexture &) = delete;

public:
	GLsizei Width() const noexcept { return width; }
	GLsizei Height() const noexcept { return height; }
	GLsizei Depth() const noexcept { return depth; }

	void Bind() noexcept;

	void Reserve(GLsizei w, GLsizei h, GLsizei d, const Format &) noexcept;
	void Data(GLsizei l, const SDL_Surface &);
	void Data(GLsizei l, const Format &, GLvoid *data) noexcept;

	void FilterNearest() noexcept;
	void FilterLinear() noexcept;
	void FilterTrilinear() noexcept;

private:
	GLuint handle;

	GLsizei width, height, depth;

	Format format;

};

}

#endif