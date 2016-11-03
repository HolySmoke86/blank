#ifndef BLANK_UI_TEXTINPUT_HPP_
#define BLANK_UI_TEXTINPUT_HPP_

#include "Text.hpp"
#include "../graphics/PrimitiveMesh.hpp"

#include <string>
#include <SDL.h>


namespace blank {

class Viewport;

class TextInput {

public:
	explicit TextInput(const Font &);

	const std::string &GetInput() const noexcept { return input; }

	void Focus(Viewport &) noexcept;
	void Blur() noexcept;

	void Clear() noexcept;
	void Backspace() noexcept;
	void Delete() noexcept;

	void MoveBegin() noexcept;
	void MoveBackward() noexcept;
	void MoveForward() noexcept;
	void MoveEnd() noexcept;

	void Insert(const char *);

	bool AtBegin() const noexcept;
	bool AtEnd() const noexcept;

	void Position(const glm::vec3 &p, Gravity g, Gravity pv) noexcept;
	void Width(float) noexcept;

	void Foreground(const PrimitiveMesh::Color &col) noexcept { fg = col; dirty_cursor = true; }
	void Background(const PrimitiveMesh::Color &col) noexcept { bg = col; dirty_box = true; }

	void Handle(const SDL_TextInputEvent &);
	void Handle(const SDL_TextEditingEvent &);

	void Render(Viewport &);

private:
	void Refresh();

private:
	const Font &font;
	std::string input;
	std::string::size_type cursor;
	Text text;

	PrimitiveMesh bg_mesh;
	PrimitiveMesh cursor_mesh;

	PrimitiveMesh::Color bg;
	PrimitiveMesh::Color fg;

	glm::vec3 position;
	glm::vec2 size;
	Gravity gravity;

	bool active;
	bool dirty_box;
	bool dirty_cursor;
	bool dirty_text;

};

}

#endif
