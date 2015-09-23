#ifndef BLANK_GRAPHICS_FORMAT_HPP_
#define BLANK_GRAPHICS_FORMAT_HPP_

#include <SDL.h>
#include <GL/glew.h>


namespace blank {

struct Format {

	GLenum format;
	GLenum type;
	GLenum internal;

	SDL_PixelFormat sdl_format;

	Format() noexcept;
	explicit Format(const SDL_PixelFormat &) noexcept;

	bool Compatible(const Format &other) const noexcept;

};

}

#endif
