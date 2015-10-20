#ifndef BLANK_UI_MESSAGEBOX_HPP_
#define BLANK_UI_MESSAGEBOX_HPP_

#include "Text.hpp"
#include "../graphics/align.hpp"
#include "../graphics/PrimitiveMesh.hpp"

#include <deque>
#include <string>
#include <glm/glm.hpp>


namespace blank {

class Font;
class Viewport;

class MessageBox {

public:
	explicit MessageBox(const Font &);

	void Position(const glm::vec3 &, Gravity) noexcept;

	void Foreground(const glm::vec4 &col) noexcept { fg = col; }
	void Background(const glm::vec4 &col) noexcept { bg = col; dirty = true; }

	void PushLine(const char *);
	void PushLine(const std::string &l) {
		PushLine(l.c_str());
	}

	void Render(Viewport &) noexcept;

private:
	void Recalc();

private:
	const Font &font;
	std::deque<Text> lines;
	std::size_t max_lines;

	glm::vec3 pos;
	glm::vec3 adv;
	glm::vec2 size;

	glm::vec4 bg;
	glm::vec4 fg;

	PrimitiveMesh bg_mesh;

	Gravity grav;
	bool dirty;

};

}

#endif
