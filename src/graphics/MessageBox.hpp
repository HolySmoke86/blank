#ifndef BLANK_GRAPHICS_MESSAGEBOX_HPP_
#define BLANK_GRAPHICS_MESSAGEBOX_HPP_

#include "align.hpp"
#include "Text.hpp"

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
	void Background(const glm::vec4 &col) noexcept { bg = col; }

	void PushLine(const char *);
	void PushLine(const std::string &l) {
		PushLine(l.c_str());
	}

	void Render(Viewport &) noexcept;

private:
	const Font &font;
	std::deque<Text> lines;
	std::size_t max_lines;

	glm::vec3 pos;
	glm::vec3 adv;

	glm::vec4 bg;
	glm::vec4 fg;

	Gravity grav;

};

}

#endif
