#include "FixedText.hpp"
#include "MessageBox.hpp"
#include "Progress.hpp"
#include "Text.hpp"
#include "TextInput.hpp"

#include "../graphics/Font.hpp"
#include "../graphics/Viewport.hpp"

#include <cstdio>
#include <cstring>
#include <limits>

using namespace std;


namespace blank {

MessageBox::MessageBox(const Font &f)
: font(f)
, lines()
, max_lines(10)
, pos(0.0f)
, adv(0.0f, font.LineSkip(), 0.0f)
, bg(1.0f, 1.0f, 1.0f, 0.0f)
, fg(1.0f, 1.0f, 1.0f, 1.0f)
, grav(Gravity::NORTH_WEST)
, dirty(true) {

}

void MessageBox::Position(const glm::vec3 &p, Gravity g) noexcept {
	pos = p;
	grav = g;
	if (get_y(g) == Align::END) {
		adv.y = -font.LineSkip();
	} else {
		adv.y = font.LineSkip();
	}
	for (Text &txt : lines) {
		txt.Pivot(g);
	}
	dirty = true;
}

void MessageBox::PushLine(const char *text) {
	lines.emplace_front();
	Text &txt = lines.front();
	txt.Set(font, text);
	txt.Pivot(grav);

	while (lines.size() > max_lines) {
		lines.pop_back();
	}
	dirty = true;
}

namespace {

PrimitiveMesh::Buffer bg_buf;

}

void MessageBox::Recalc() {
	size = glm::vec2(0.0f, 0.0f);
	for (const Text &line : lines) {
		size.x = max(size.x, line.Size().x);
		size.y += line.Size().y;
	}
	bg_buf.FillRect(size.x, size.y, bg, align(grav, size));
	bg_mesh.Update(bg_buf);
	bg_buf.Clear();
	dirty = false;
}

void MessageBox::Render(Viewport &viewport) noexcept {
	viewport.SetCursor(pos, grav);
	if (bg.a > numeric_limits<float>::epsilon()) {
		if (dirty) {
			Recalc();
		}
		PlainColor &prog = viewport.HUDColorProgram();
		prog.SetM(viewport.Cursor());
		bg_mesh.DrawTriangles();
		viewport.MoveCursor(glm::vec3(0.0f, 0.0f, -1.0f));
	}
	BlendedSprite &prog = viewport.SpriteProgram();
	prog.SetBG(glm::vec4(0.0f));
	prog.SetFG(fg);
	for (Text &txt : lines) {
		prog.SetM(viewport.Cursor());
		txt.Render(viewport);
		viewport.MoveCursor(adv);
	}
}


Progress::Progress(Font &font) noexcept
: font(font)
, text()
, tpl("%d/%d (%d%%)") {

}

namespace {

char buf[128] = { '\0' };

}

void Progress::Update(int current, int total) {
	snprintf(buf, sizeof(buf), tpl, current, total, current * 100 / total);
	text.Set(font, buf);
}

void Progress::Render(Viewport &viewport) noexcept {
	text.Render(viewport);
}


Text::Text() noexcept
: tex()
, sprite()
, size(0.0f)
, pivot(Gravity::NORTH_WEST)
, dirty(false) {

}

FixedText::FixedText() noexcept
: Text()
, bg(1.0f, 1.0f, 1.0f, 0.0f)
, fg(1.0f, 1.0f, 1.0f, 1.0f)
, pos(0.0f)
, grav(Gravity::NORTH_WEST)
, visible(false) {

}

void Text::Set(const Font &font, const char *text) {
	font.Render(text, tex);
	size = font.TextSize(text);
	dirty = true;
}

namespace {

SpriteMesh::Buffer sprite_buf;

}

void Text::Update() {
	sprite_buf.LoadRect(size.x, size.y, align(pivot, size));
	sprite.Update(sprite_buf);
	dirty = false;
}

void FixedText::Render(Viewport &viewport) noexcept {
	BlendedSprite &prog = viewport.SpriteProgram();
	viewport.SetCursor(pos, grav);
	prog.SetM(viewport.Cursor());
	prog.SetBG(bg);
	prog.SetFG(fg);
	Text::Render(viewport);
}

void Text::Render(Viewport &viewport) noexcept {
	if (dirty) {
		Update();
	}
	BlendedSprite &prog = viewport.SpriteProgram();
	prog.SetTexture(tex);
	sprite.Draw();
}


TextInput::TextInput(const Font &font)
: font(font)
, input()
, cursor(0)
, text()
, bg_mesh()
, cursor_mesh()
, bg(1.0f, 1.0f, 1.0f, 0.0f)
, fg(1.0f, 1.0f, 1.0f, 1.0f)
, position(0.0f)
, size(font.LineSkip())
, gravity(Gravity::NORTH_WEST)
, active(false)
, dirty_box(true)
, dirty_cursor(true)
, dirty_text(true) {

}

void TextInput::Focus(Viewport &viewport) noexcept {
	SDL_StartTextInput();
	active = true;

	glm::vec2 p(viewport.GetPosition(glm::vec2(position), gravity));
	SDL_Rect rect;
	rect.x = p.x;
	rect.y = p.y;
	rect.w = size.x;
	rect.h = size.y;
	SDL_SetTextInputRect(&rect);
}

void TextInput::Blur() noexcept {
	SDL_StopTextInput();
	active = false;
}

void TextInput::Clear() noexcept {
	input.clear();
	cursor = 0;
	dirty_text = true;
}

void TextInput::Backspace() noexcept {
	string::size_type previous(cursor);
	MoveBackward();
	input.erase(cursor, previous - cursor);
	dirty_text = true;
}

void TextInput::Delete() noexcept {
	string::size_type previous(cursor);
	MoveForward();
	input.erase(previous, cursor - previous);
	cursor = previous;
	dirty_text = true;
}

void TextInput::MoveBegin() noexcept {
	cursor = 0;
}

void TextInput::MoveBackward() noexcept {
	if (AtBegin()) return;
	--cursor;
	while (cursor > 0 && (input[cursor] & 0xC0) == 0x80) {
		--cursor;
	}
}

void TextInput::MoveForward() noexcept {
	if (AtEnd()) return;
	++cursor;
	while (cursor <= input.size() && (input[cursor] & 0xC0) == 0x80) {
		++cursor;
	}
}

void TextInput::MoveEnd() noexcept {
	cursor = input.size();
}

void TextInput::Insert(const char *str) {
	size_t len = strlen(str);
	input.insert(cursor, str, len);
	cursor += len;
	dirty_text = true;
}

bool TextInput::AtBegin() const noexcept {
	return cursor == 0;
}

bool TextInput::AtEnd() const noexcept {
	return cursor == input.size();
}

void TextInput::Position(const glm::vec3 &p, Gravity g, Gravity pv) noexcept {
	position = p;
	gravity = g;
	text.Pivot(pv);
	dirty_box = true;
}

void TextInput::Width(float w) noexcept {
	size.x = w;
	dirty_box = true;
}

void TextInput::Handle(const SDL_TextInputEvent &e) {
	Insert(e.text);
}

void TextInput::Handle(const SDL_TextEditingEvent &e) {
}

void TextInput::Refresh() {
	if (dirty_box) {
		bg_buf.FillRect(size.x, size.y, bg, align(gravity, size));
		bg_mesh.Update(bg_buf);
		bg_buf.Clear();
		dirty_box = false;
	}
	if (dirty_cursor) {
		bg_buf.Reserve(2, 2);
		bg_buf.vertices.emplace_back(0.0f, 0.0f, 0.0f);
		bg_buf.vertices.emplace_back(0.0f, float(font.LineSkip()), 0.0f);
		bg_buf.colors.resize(2, fg);
		bg_buf.indices.push_back(0);
		bg_buf.indices.push_back(1);
		cursor_mesh.Update(bg_buf);
		bg_buf.Clear();
		dirty_cursor = false;
	}
	if (dirty_text) {
		if (!input.empty()) {
			text.Set(font, input.c_str());
		}
		dirty_text = false;
	}
}

void TextInput::Render(Viewport &viewport) {
	Refresh();
	viewport.SetCursor(position, gravity);
	if (bg.a > numeric_limits<float>::epsilon()) {
		viewport.EnableAlphaBlending();
		PlainColor &prog = viewport.HUDColorProgram();
		prog.SetM(viewport.Cursor());
		bg_mesh.DrawTriangles();
		viewport.MoveCursor(glm::vec3(0.0f, 0.0f, -1.0f));
	}
	if (!input.empty()) {
		BlendedSprite &prog = viewport.SpriteProgram();
		prog.SetBG(glm::vec4(0.0f));
		prog.SetFG(fg);
		prog.SetM(viewport.Cursor());
		text.Render(viewport);
	}
	if (active) {
		glm::vec2 offset(0.0f);
		if (input.empty() || AtBegin()) {
			// a okay
			offset = -align(text.Pivot(), glm::vec2(0.0f, font.LineSkip()));
		} else if (AtEnd()) {
			offset = -align(text.Pivot(), text.Size(), glm::vec2(-text.Size().x, 0.0f));
		} else {
			offset = -align(text.Pivot(), text.Size(), glm::vec2(-font.TextSize(input.substr(0, cursor).c_str()).x, 0.0f));
		}
		viewport.MoveCursor(glm::vec3(offset, -1.0f));
		PlainColor &prog = viewport.HUDColorProgram();
		prog.SetM(viewport.Cursor());
		cursor_mesh.DrawLines();
	}
}

}
