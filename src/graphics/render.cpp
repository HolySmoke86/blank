#include "BlendedSprite.hpp"
#include "FixedText.hpp"
#include "Font.hpp"
#include "Format.hpp"
#include "MessageBox.hpp"
#include "Text.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"

#include <algorithm>
#include <cstring>
#include <memory>
#include <stdexcept>


namespace blank {

Font::Font(const char *src, int size, long index)
: handle(TTF_OpenFontIndex(src, size, index)) {
	if (!handle) {
		throw std::runtime_error(TTF_GetError());
	}
}

Font::~Font() {
	if (handle) {
		TTF_CloseFont(handle);
	}
}

Font::Font(Font &&other) noexcept
: handle(other.handle) {
	other.handle = nullptr;
}

Font &Font::operator =(Font &&other) noexcept {
	std::swap(handle, other.handle);
	return *this;
}


int Font::Style() const noexcept {
	return TTF_GetFontStyle(handle);
}

void Font::Style(int s) const noexcept {
	TTF_SetFontStyle(handle, s);
}

int Font::Outline() const noexcept {
	return TTF_GetFontOutline(handle);
}

void Font::Outline(int px) noexcept {
	TTF_SetFontOutline(handle, px);
}


int Font::Hinting() const noexcept {
	return TTF_GetFontHinting(handle);
}

void Font::Hinting(int h) const noexcept {
	TTF_SetFontHinting(handle, h);
}

bool Font::Kerning() const noexcept {
	return TTF_GetFontKerning(handle);
}

void Font::Kerning(bool b) noexcept {
	TTF_SetFontKerning(handle, b);
}


int Font::Height() const noexcept {
	return TTF_FontHeight(handle);
}

int Font::Ascent() const noexcept {
	return TTF_FontAscent(handle);
}

int Font::Descent() const noexcept {
	return TTF_FontDescent(handle);
}

int Font::LineSkip() const noexcept {
	return TTF_FontLineSkip(handle);
}


const char *Font::FamilyName() const noexcept {
	return TTF_FontFaceFamilyName(handle);
}

const char *Font::StyleName() const noexcept {
	return TTF_FontFaceStyleName(handle);
}


bool Font::HasGlyph(Uint16 c) const noexcept {
	return TTF_GlyphIsProvided(handle, c);
}


glm::tvec2<int> Font::TextSize(const char *text) const {
	glm::tvec2<int> size;
	if (TTF_SizeUTF8(handle, text, &size.x, &size.y) != 0) {
		throw std::runtime_error(TTF_GetError());
	}
	return size;
}

Texture Font::Render(const char *text) const {
	Texture tex;
	Render(text, tex);
	return tex;
}

void Font::Render(const char *text, Texture &tex) const {
	SDL_Surface *srf = TTF_RenderUTF8_Blended(handle, text, { 0xFF, 0xFF, 0xFF, 0xFF });
	if (!srf) {
		throw std::runtime_error(TTF_GetError());
	}
	tex.Bind();
	tex.Data(*srf, false);
	tex.FilterLinear();
	SDL_FreeSurface(srf);
}


void Format::ReadPixelFormat(const SDL_PixelFormat &fmt) {
	if (fmt.BytesPerPixel == 4) {
		if (fmt.Amask == 0xFF) {
			if (fmt.Rmask == 0xFF00) {
				format = GL_BGRA;
			} else {
				format = GL_RGBA;
			}
			type = GL_UNSIGNED_INT_8_8_8_8;
		} else {
			if (fmt.Rmask == 0xFF) {
				format = GL_RGBA;
			} else {
				format = GL_BGRA;
			}
			type = GL_UNSIGNED_INT_8_8_8_8_REV;
		}
		internal = GL_RGBA8;
	} else {
		if (fmt.Rmask == 0xFF) {
			format = GL_RGB;
		} else {
			format = GL_BGR;
		}
		type = GL_UNSIGNED_BYTE;
		internal = GL_RGB8;
	}
}


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


Texture::Texture()
: handle(0)
, width(0)
, height(0) {
	glGenTextures(1, &handle);
}

Texture::~Texture() {
	if (handle != 0) {
		glDeleteTextures(1, &handle);
	}
}

Texture::Texture(Texture &&other) noexcept
: handle(other.handle) {
	other.handle = 0;
	width = other.width;
	height = other.height;
}

Texture &Texture::operator =(Texture &&other) noexcept {
	std::swap(handle, other.handle);
	width = other.width;
	height = other.height;
	return *this;
}


void Texture::Bind() noexcept {
	glBindTexture(GL_TEXTURE_2D, handle);
}

namespace {
	bool ispow2(unsigned int i) {
		// don't care about i == 0 here
		return !(i & (i - 1));
	}
}

void Texture::Data(const SDL_Surface &srf, bool pad2) noexcept {
	Format format;
	format.ReadPixelFormat(*srf.format);

	if (!pad2 || (ispow2(srf.w) && ispow2(srf.h))) {
		int align = UnpackAlignmentFromPitch(srf.pitch);

		int pitch = (srf.w * srf.format->BytesPerPixel + align - 1) / align * align;
		if (srf.pitch - pitch >= align) {
			UnpackRowLength(srf.pitch / srf.format->BytesPerPixel);
		} else {
			UnpackRowLength(0);
		}

		Data(srf.w, srf.h, format, srf.pixels);

		UnpackRowLength(0);
	} else if (srf.w > (1 << 30) || srf.h > (1 << 30)) {
#ifndef NDEBUG
		throw std::runtime_error("texture too large");
#endif
	} else {
		GLsizei width = 1, height = 1;
		while (width < srf.w) {
			width <<= 1;
		}
		while (height < srf.h) {
			height <<= 1;
		}
		size_t pitch = width * srf.format->BytesPerPixel;
		size_t size = pitch * height;
		size_t row_pad = pitch - srf.pitch;
		std::unique_ptr<unsigned char[]> data(new unsigned char[size]);
		unsigned char *src = reinterpret_cast<unsigned char *>(srf.pixels);
		unsigned char *dst = data.get();
		for (int row = 0; row < srf.h; ++row) {
			std::memcpy(dst, src, srf.pitch);
			src += srf.pitch;
			dst += srf.pitch;
			std::memset(dst, 0, row_pad);
			dst += row_pad;
		}
		std::memset(dst, 0, (height - srf.h) * pitch);
		UnpackAlignmentFromPitch(pitch);
		Data(width, height, format, data.get());
	}

	UnpackAlignment(4);
}

void Texture::Data(GLsizei w, GLsizei h, const Format &format, GLvoid *data) noexcept {
	glTexImage2D(
		GL_TEXTURE_2D,
		0, format.internal,
		w, h,
		0,
		format.format, format.type,
		data
	);
	width = w;
	height = h;
}


void Texture::FilterNearest() noexcept {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Texture::FilterLinear() noexcept {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::FilterTrilinear() noexcept {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}


void Texture::UnpackAlignment(GLint i) noexcept {
	glPixelStorei(GL_UNPACK_ALIGNMENT, i);
}

int Texture::UnpackAlignmentFromPitch(int pitch) noexcept {
	int align = 8;
	while (pitch % align) {
		align >>= 1;
	}
	UnpackAlignment(align);
	return align;
}

void Texture::UnpackRowLength(GLint i) noexcept {
	glPixelStorei(GL_UNPACK_ROW_LENGTH, i);
}

}
