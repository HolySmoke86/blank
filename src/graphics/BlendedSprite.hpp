#ifndef BLANK_GRAPHICS_BLENDEDSPRITE_HPP_
#define BLANK_GRAPHICS_BLENDEDSPRITE_HPP_

#include "glm.hpp"
#include "Program.hpp"

#include <GL/glew.h>


namespace blank {

class Texture;

class BlendedSprite {

public:
	BlendedSprite();

	void Activate() noexcept;

	void SetM(const glm::mat4 &m) noexcept;
	void SetProjection(const glm::mat4 &p) noexcept;
	void SetView(const glm::mat4 &v) noexcept;
	void SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept;

	void SetTexture(Texture &) noexcept;
	void SetFG(const glm::vec4 &) noexcept;
	void SetBG(const glm::vec4 &) noexcept;

	const glm::mat4 &Projection() const noexcept { return projection; }
	const glm::mat4 &View() const noexcept { return view; }
	const glm::mat4 &GetVP() const noexcept { return vp; }

private:
	Program program;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 vp;

	GLuint mvp_handle;
	GLuint sampler_handle;
	GLuint fg_handle;
	GLuint bg_handle;

};

}

#endif
