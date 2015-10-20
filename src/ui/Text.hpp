#ifndef BLANK_UI_TEXT_HPP_
#define BLANK_UI_TEXT_HPP_

#include "../graphics/align.hpp"
#include "../graphics/Texture.hpp"
#include "../graphics/SpriteMesh.hpp"

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

	const glm::vec2 &Size() const noexcept { return size; }

	void Render(Viewport &) noexcept;

private:
	void Update();

private:
	Texture tex;
	SpriteMesh sprite;
	glm::vec2 size;
	Gravity pivot;
	bool dirty;

};

}

#endif
