#ifndef BLANK_GRAPHICS_FORMAT_HPP_
#define BLANK_GRAPHICS_FORMAT_HPP_

#include <SDL.h>
#include <GL/glew.h>


namespace blank {

struct Format {

	GLenum format;
	GLenum type;
	GLenum internal;

	void ReadPixelFormat(const SDL_PixelFormat &);

};

}

#endif
