#ifndef BLANK_SHADER_HPP_
#define BLANK_SHADER_HPP_

#include <iosfwd>
#include <list>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class Shader {

public:
	explicit Shader(GLenum type);
	~Shader();

	Shader(Shader &&);
	Shader &operator =(Shader &&);

	Shader(const Shader &) = delete;
	Shader &operator =(const Shader &) = delete;

	void Source(const GLchar *src);
	void Compile();
	bool Compiled() const;
	void Log(std::ostream &) const;

	void AttachToProgram(GLuint id) const;

private:
	GLuint handle;

};


class Program {

public:
	Program();
	~Program();

	Program(const Program &) = delete;
	Program &operator =(const Program &) = delete;

	const Shader &LoadShader(GLenum type, const GLchar *src);
	void Attach(Shader &);
	void Link();
	bool Linked() const;
	void Log(std::ostream &) const;

	GLint UniformLocation(const GLchar *name) const;

	void Use() const { glUseProgram(handle); }

private:
	GLuint handle;
	std::list<Shader> shaders;

};


class DirectionalLighting {

public:
	DirectionalLighting();

	void Activate();

	void SetM(const glm::mat4 &m);
	void SetVP(const glm::mat4 &v, const glm::mat4 &p);
	void SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p);

private:
	Program program;

	glm::vec3 light_direction;
	glm::vec3 light_color;

	glm::mat4 vp;

	GLuint m_handle;
	GLuint mvp_handle;
	GLuint light_direction_handle;
	GLuint light_color_handle;

};

}

#endif
