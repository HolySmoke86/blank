#ifndef BLANK_GRAPHICS_TEXT_HPP_
#define BLANK_GRAPHICS_TEXT_HPP_

#include "align.hpp"
#include "Texture.hpp"
#include "../model/SpriteModel.hpp"

#include <string>
#include <glm/glm.hpp>


namespace blank {

class Font;
class Viewport;

class Text {

public:
	Text() noexcept;

	void Set(const Font &, const char *);
	void Set(const Font &f, const std::string &s) {
		Set(f, s.c_str());
	}

	void Position(const glm::vec3 &p) noexcept {
		pos = p;
	}
	void Position(
		const glm::vec3 &p,
		Gravity g
	) noexcept {
		pos = p;
		grav = g;
		pivot = g;
		dirty = true;
	}
	void Position(
		const glm::vec3 &p,
		Gravity g,
		Gravity pv
	) noexcept {
		pos = p;
		grav = g;
		pivot = pv;
		dirty = true;
	}

	void Foreground(const glm::vec4 &col) noexcept { fg = col; }
	void Background(const glm::vec4 &col) noexcept { bg = col; }

	void Render(Viewport &) noexcept;

	void Show() noexcept { visible = true; }
	void Hide() noexcept { visible = false; }
	void Toggle() noexcept { visible = !visible; }
	bool Visible() const noexcept { return visible; }

private:
	void Update();

private:
	Texture tex;
	SpriteModel sprite;
	glm::vec4 bg;
	glm::vec4 fg;
	glm::vec2 size;
	glm::vec3 pos;
	Gravity grav;
	Gravity pivot;
	bool dirty;
	bool visible;

};

}

#endif
