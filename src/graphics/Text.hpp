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

	void Pivot(Gravity p) {
		pivot = p;
		dirty = true;
	}

	void Render(Viewport &) noexcept;

private:
	void Update();

private:
	Texture tex;
	SpriteModel sprite;
	glm::vec2 size;
	Gravity pivot;
	bool dirty;

};

}

#endif
