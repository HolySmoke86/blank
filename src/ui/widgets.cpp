#include "FixedText.hpp"
#include "MessageBox.hpp"
#include "Progress.hpp"
#include "Text.hpp"

#include "../graphics/Font.hpp"
#include "../graphics/Viewport.hpp"

#include <cstdio>


namespace blank {

MessageBox::MessageBox(const Font &f)
: font(f)
, lines()
, max_lines(10)
, pos(0.0f)
, adv(0.0f, font.LineSkip(), 0.0f)
, bg(1.0f, 1.0f, 1.0f, 0.0f)
, fg(1.0f, 1.0f, 1.0f, 1.0f)
, grav(Gravity::NORTH_WEST) {

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
}

void MessageBox::PushLine(const char *text) {
	lines.emplace_front();
	Text &txt = lines.front();
	txt.Set(font, text);
	txt.Pivot(grav);

	while (lines.size() > max_lines) {
		lines.pop_back();
	}
}

void MessageBox::Render(Viewport &viewport) noexcept {
	BlendedSprite &prog = viewport.SpriteProgram();
	prog.SetBG(bg);
	prog.SetFG(fg);
	viewport.SetCursor(pos, grav);
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
	std::snprintf(buf, sizeof(buf), tpl, current, total, current * 100 / total);
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

SpriteModel::Buffer sprite_buf;

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

}
