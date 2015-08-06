#ifndef BLANK_UI_PROGRESS_HPP_
#define BLANK_UI_PROGRESS_HPP_

#include "FixedText.hpp"


namespace blank {

class Font;
class Viewport;

class Progress {

public:
	explicit Progress(Font &) noexcept;

	void Position(const glm::vec3 &p, Gravity g) noexcept { text.Position(p, g); }
	void Template(const char *t) noexcept { tpl = t; }

	void Update(int current, int total);
	void Render(Viewport &) noexcept;

private:
	Font &font;
	FixedText text;
	const char *tpl;

};

}

#endif
