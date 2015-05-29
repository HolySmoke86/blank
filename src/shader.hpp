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

	Shader(Shader &&) noexcept;
	Shader &operator =(Shader &&) noexcept;

	Shader(const Shader &) = delete;
	Shader &operator =(const Shader &) = delete;

	void Source(const GLchar *src) noexcept;
	void Compile() noexcept;
	bool Compiled() const noexcept;
	void Log(std::ostream &) const;

	void AttachToProgram(GLuint id) const noexcept;

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


class DirectionalLighting {

public:
	DirectionalLighting();

	void Activate() noexcept;

	void SetLightDirection(const glm::vec3 &) noexcept;

	void SetFogDensity(float) noexcept;

	void SetM(const glm::mat4 &m) noexcept;
	void SetProjection(const glm::mat4 &p) noexcept;
	void SetView(const glm::mat4 &v) noexcept;
	void SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept;

	const glm::mat4 &Projection() const noexcept { return projection; }
	const glm::mat4 &View() const noexcept { return view; }
	const glm::mat4 &GetVP() const noexcept { return vp; }

private:
	Program program;

	glm::vec3 light_direction;
	glm::vec3 light_color;

	float fog_density;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 vp;

	GLuint m_handle;
	GLuint mv_handle;
	GLuint mvp_handle;
	GLuint light_direction_handle;
	GLuint light_color_handle;
	GLuint fog_density_handle;

};

class BlockLighting {

public:
	BlockLighting();

	void Activate() noexcept;

	void SetFogDensity(float) noexcept;

	void SetM(const glm::mat4 &m) noexcept;
	void SetProjection(const glm::mat4 &p) noexcept;
	void SetView(const glm::mat4 &v) noexcept;
	void SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept;
	void SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept;

	const glm::mat4 &Projection() const noexcept { return projection; }
	const glm::mat4 &View() const noexcept { return view; }
	const glm::mat4 &GetVP() const noexcept { return vp; }

private:
	Program program;

	float fog_density;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 vp;

	GLuint mv_handle;
	GLuint mvp_handle;
	GLuint light_direction_handle;
	GLuint light_color_handle;
	GLuint fog_density_handle;

};

}

#endif
