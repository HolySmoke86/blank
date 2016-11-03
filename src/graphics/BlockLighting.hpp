#ifndef BLANK_GRAPHICS_BLOCKLIGHTING_HPP_
#define BLANK_GRAPHICS_BLOCKLIGHTING_HPP_

#include "glm.hpp"
#include "Program.hpp"

#include <GL/glew.h>


namespace blank {

class ArrayTexture;

class BlockLighting {

public:
	BlockLighting();

	void Activate() noexcept;

	void SetTexture(ArrayTexture &) noexcept;
	void SetFogDensity(float) noexcept;

	void SetM(const glm::mat4 &m) noexcept;
	void SetProjection(const glm::mat4 &p) noexcept;
	void SetView(const glm::mat4 &v) noexcept;
	void SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept;

	const glm::mat4 &Projection() const noexcept { return projection; }
	const glm::mat4 &View() const noexcept { return view; }
	const glm::mat4 &GetVP() const noexcept { return vp; }

private:
	Program program;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 vp;

	GLuint mv_handle;
	GLuint mvp_handle;
	GLuint sampler_handle;
	GLuint light_direction_handle;
	GLuint light_color_handle;
	GLuint fog_density_handle;

};

}

#endif
