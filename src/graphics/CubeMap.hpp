#ifndef BLANK_GRAPHICS_CUBEMAP_HPP_
#define BLANK_GRAPHICS_CUBEMAP_HPP_

#include "Format.hpp"
#include "TextureBase.hpp"

#include <GL/glew.h>

struct SDL_Surface;


namespace blank {

class CubeMap
: public TextureBase<GL_TEXTURE_CUBE_MAP> {

public:
	enum Face {
		RIGHT = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		LEFT = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		TOP = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		BOTTOM = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		BACK = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		FRONT = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	};

public:
	CubeMap();
	~CubeMap();

	CubeMap(CubeMap &&) noexcept;
	CubeMap &operator =(CubeMap &&) noexcept;

	CubeMap(const CubeMap &) = delete;
	CubeMap &operator =(const CubeMap &) = delete;

public:
	void Data(Face, const SDL_Surface &) noexcept;
	void Data(Face, GLsizei w, GLsizei h, const Format &, GLvoid *data) noexcept;

};

}

#endif
