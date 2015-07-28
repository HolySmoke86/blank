#ifndef BLANK_GRAPHICS_FIXEDTEXT_HPP_
#define BLANK_GRAPHICS_FIXEDTEXT_HPP_

#include "Text.hpp"


namespace blank {

class FixedText
: public Text {

public:
	FixedText() noexcept;

	void Position(const glm::vec3 &p) noexcept {
		pos = p;
	}
	void Position(
		const glm::vec3 &p,
		Gravity g
	) noexcept {
		pos = p;
		grav = g;
		Pivot(g);
	}
	void Position(
		const glm::vec3 &p,
		Gravity g,
		Gravity pv
	) noexcept {
		pos = p;
		grav = g;
		Pivot(pv);
	}

	void Foreground(const glm::vec4 &col) noexcept { fg = col; }
	void Background(const glm::vec4 &col) noexcept { bg = col; }

	void Show() noexcept { visible = true; }
	void Hide() noexcept { visible = false; }
	void Toggle() noexcept { visible = !visible; }
	bool Visible() const noexcept { return visible; }

	void Render(Viewport &) noexcept;

private:
	glm::vec4 bg;
	glm::vec4 fg;
	glm::vec3 pos;
	Gravity grav;
	bool visible;

};

}

#endif
