#ifndef BLANK_GRAPHICS_CANVAS_HPP_
#define BLANK_GRAPHICS_CANVAS_HPP_

#include "glm.hpp"


namespace blank {

class Canvas {

public:
	Canvas() noexcept;

	void Resize(float w, float h) noexcept;

	const glm::vec2 &Offset() const noexcept { return offset; }
	const glm::vec2 &Size() const noexcept { return size; }

	const glm::mat4 &Projection() const noexcept { return projection; }
	const glm::mat4 &View() const noexcept { return view; }

private:
	void UpdateProjection() noexcept;

private:
	glm::vec2 offset;
	glm::vec2 size;
	float near;
	float far;

	glm::mat4 projection;
	glm::mat4 view;

};

}

#endif
