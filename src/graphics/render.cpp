#include "ArrayTexture.hpp"
#include "Font.hpp"
#include "Format.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"

#include "../app/init.hpp"

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


glm::ivec2 Font::TextSize(const char *text) const {
	glm::ivec2 size;
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

Format::Format()
: format(GL_BGRA)
, type(GL_UNSIGNED_INT_8_8_8_8_REV)
, internal(GL_RGBA8) {
	sdl_format.format = SDL_PIXELFORMAT_ARGB8888;
	sdl_format.palette = nullptr;
	sdl_format.BitsPerPixel = 32;
	sdl_format.BytesPerPixel = 4;
	sdl_format.Rmask = 0x00FF0000;
	sdl_format.Gmask = 0x0000FF00;
	sdl_format.Bmask = 0x000000FF;
	sdl_format.Amask = 0xFF000000;
	sdl_format.Rloss = 0;
	sdl_format.Gloss = 0;
	sdl_format.Bloss = 0;
	sdl_format.Aloss = 0;
	sdl_format.Rshift = 16;
	sdl_format.Gshift = 8;
	sdl_format.Bshift = 0;
	sdl_format.Ashift = 24;
	sdl_format.refcount = 1;
	sdl_format.next = nullptr;
}

Format::Format(const SDL_PixelFormat &fmt)
: sdl_format(fmt) {
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

bool Format::Compatible(const Format &other) const noexcept {
	return format == other.format && type == other.type && internal == other.internal;
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
	Format format(*srf.format);

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


ArrayTexture::ArrayTexture()
: handle(0)
, width(0)
, height(0)
, depth(0) {
	glGenTextures(1, &handle);
}

ArrayTexture::~ArrayTexture() {
	if (handle != 0) {
		glDeleteTextures(1, &handle);
	}
}

ArrayTexture::ArrayTexture(ArrayTexture &&other) noexcept
: handle(other.handle) {
	other.handle = 0;
	width = other.width;
	height = other.height;
	depth = other.depth;
}

ArrayTexture &ArrayTexture::operator =(ArrayTexture &&other) noexcept {
	std::swap(handle, other.handle);
	width = other.width;
	height = other.height;
	depth = other.depth;
	return *this;
}


void ArrayTexture::Bind() noexcept {
	glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
}


void ArrayTexture::Reserve(GLsizei w, GLsizei h, GLsizei d, const Format &f) noexcept {
	glTexStorage3D(
		GL_TEXTURE_2D_ARRAY, // which
		1,                   // mipmap count
		f.internal,          // format
		w, h,                // dimensions
		d                    // layer count
	);
	width = w;
	height = h;
	depth = d;
	format = f;
}

void ArrayTexture::Data(GLsizei l, const SDL_Surface &srf) {
	Format fmt(*srf.format);
	if (format.Compatible(fmt)) {
		Data(l, fmt, srf.pixels);
	} else {
		SDL_Surface *converted = SDL_ConvertSurface(
			const_cast<SDL_Surface *>(&srf),
			&format.sdl_format,
			0
		);
		if (!converted) {
			throw SDLError("SDL_ConvertSurface");
		}
		Format new_fmt(*converted->format);
		if (!format.Compatible(new_fmt)) {
			SDL_FreeSurface(converted);
			throw std::runtime_error("unable to convert texture input");
		}
		Data(l, new_fmt, converted->pixels);
		SDL_FreeSurface(converted);
	}
}

void ArrayTexture::Data(GLsizei l, const Format &f, GLvoid *data) noexcept {
	glTexSubImage3D(
		GL_TEXTURE_2D_ARRAY, // which
		0,                   // mipmap lavel
		0, 0,                // dest X and Y offset
		l,                   // layer offset
		width, height,
		1,                   // layer count
		f.format, f.type,
		data
	);
}


void ArrayTexture::FilterNearest() noexcept {
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void ArrayTexture::FilterLinear() noexcept {
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void ArrayTexture::FilterTrilinear() noexcept {
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

}
