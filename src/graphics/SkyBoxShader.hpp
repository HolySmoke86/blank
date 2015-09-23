#ifndef BLANK_GRAPHICS_SKYBOXSHADER_HPP_
#define BLANK_GRAPHICS_SKYBOXSHADER_HPP_

#include "CubeMap.hpp"


namespace blank {

class SkyBoxShader {

public:
	SkyBoxShader();

	void Activate() noexcept;

	void SetTexture(CubeMap &) noexcept;

	void SetProjection(const glm::mat4 &p) noexcept;
	void SetView(const glm::mat4 &v) noexcept;
	void SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept;

	const glm::mat4 &Projection() const noexcept { return projection; }
	const glm::mat4 &View() const noexcept { return view; }
	const glm::mat4 &GetVP() const noexcept { return vp; }

private:
	Program program;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 vp;

	GLuint vp_handle;
	GLuint sampler_handle;

};

}

#endif
