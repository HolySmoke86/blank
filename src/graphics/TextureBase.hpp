#ifndef BLANK_GRAPHICS_TEXTUREBASE_HPP_
#define BLANK_GRAPHICS_TEXTUREBASE_HPP_

#include <GL/glew.h>


namespace blank {

template<GLenum TARGET, GLsizei COUNT = 1>
class TextureBase {

public:
	TextureBase();
	~TextureBase();

	TextureBase(TextureBase &&other) noexcept;
	TextureBase &operator =(TextureBase &&) noexcept;

	TextureBase(const TextureBase &) = delete;
	TextureBase &operator =(const TextureBase &) = delete;

public:
	void Bind(GLsizei which = 0) noexcept {
		glBindTexture(TARGET, handle[which]);
	}

	void FilterNearest() noexcept {
		glTexParameteri(TARGET, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(TARGET, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	void FilterLinear() noexcept {
		glTexParameteri(TARGET, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(TARGET, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	void FilterTrilinear() noexcept {
		glTexParameteri(TARGET, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(TARGET, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(TARGET);
	}

	void WrapEdge() noexcept {
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	void WrapBorder() noexcept {
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	void WrapRepeat() noexcept {
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	void WrapMirror() noexcept {
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(TARGET, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	}

private:
	GLuint handle[COUNT];

};

}

#endif
