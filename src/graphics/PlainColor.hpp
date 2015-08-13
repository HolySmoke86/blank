#ifndef BLANK_GRAPHICS_PLAINCOLOR_HPP_
#define BLANK_GRAPHICS_PLAINCOLOR_HPP_

#include "Program.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class PlainColor {

public:
	PlainColor();

	void Activate() noexcept;

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

	GLuint mvp_handle;

};

}

#endif
