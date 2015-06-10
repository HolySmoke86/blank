#ifndef BLANK_GRAPHICS_PROGRAM_HPP_
#define BLANK_GRAPHICS_PROGRAM_HPP_

#include <iosfwd>
#include <list>
#include <GL/glew.h>


namespace blank {

class Shader;

class Program {

public:
	Program();
	~Program();

	Program(const Program &) = delete;
	Program &operator =(const Program &) = delete;

	const Shader &LoadShader(GLenum type, const GLchar *src);
	void Attach(Shader &) noexcept;
	void Link() noexcept;
	bool Linked() const noexcept;
	void Log(std::ostream &) const;

	GLint AttributeLocation(const GLchar *name) const noexcept;
	GLint UniformLocation(const GLchar *name) const noexcept;

	void Use() const noexcept { glUseProgram(handle); }

private:
	GLuint handle;
	std::list<Shader> shaders;

};

}

#endif
