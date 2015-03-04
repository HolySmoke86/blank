#include "shader.hpp"

#include "init.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>


namespace {

void gl_error(std::string msg) {
	const GLubyte *errBegin = gluErrorString(glGetError());
	if (errBegin && *errBegin != '\0') {
		const GLubyte *errEnd = errBegin;
		while (*errEnd != '\0') {
			++errEnd;
		}
		msg += ": ";
		msg.append(errBegin, errEnd);
	}
	throw std::runtime_error(msg);
}

}

namespace blank {

Shader::Shader(GLenum type)
: handle(glCreateShader(type)) {
	if (handle == 0) {
		gl_error("glCreateShader");
	}
}

Shader::~Shader() {
	if (handle != 0) {
		glDeleteShader(handle);
	}
}

Shader::Shader(Shader &&other)
: handle(other.handle) {
	other.handle = 0;
}

Shader &Shader::operator =(Shader &&other) {
	std::swap(handle, other.handle);
	return *this;
}


void Shader::Source(const GLchar *src) {
	const GLchar* src_arr[] = { src };
	glShaderSource(handle, 1, src_arr, nullptr);
}

void Shader::Compile() {
	glCompileShader(handle);
}

bool Shader::Compiled() const {
	GLint compiled = GL_FALSE;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
	return compiled == GL_TRUE;
}

void Shader::Log(std::ostream &out) const {
	int log_len = 0, max_len = 0;
	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &max_len);
	std::unique_ptr<char[]> log(new char[max_len]);
	glGetShaderInfoLog(handle, max_len, &log_len, log.get());
	out.write(log.get(), log_len);
}


void Shader::AttachToProgram(GLuint id) const {
	glAttachShader(id, handle);
}


Program::Program()
: handle(glCreateProgram()) {
	if (handle == 0) {
		gl_error("glCreateProgram");
	}
}

Program::~Program() {
	if (handle != 0) {
		glDeleteProgram(handle);
	}
}


const Shader &Program::LoadShader(GLenum type, const GLchar *src) {
	shaders.emplace_back(type);
	Shader &shader = shaders.back();
	shader.Source(src);
	shader.Compile();
	if (!shader.Compiled()) {
		shader.Log(std::cerr);
		throw std::runtime_error("compile shader");
	}
	Attach(shader);
	return shader;
}

void Program::Attach(Shader &shader) {
	shader.AttachToProgram(handle);
}

void Program::Link() {
	glLinkProgram(handle);
}

bool Program::Linked() const {
	GLint linked = GL_FALSE;
	glGetProgramiv(handle, GL_LINK_STATUS, &linked);
	return linked == GL_TRUE;
}

void Program::Log(std::ostream &out) const {
	int log_len = 0, max_len = 0;
	glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &max_len);
	std::unique_ptr<char[]> log(new char[max_len]);
	glGetProgramInfoLog(handle, max_len, &log_len, log.get());
	out.write(log.get(), log_len);
}


GLint Program::UniformLocation(const GLchar *name) const {
	return glGetUniformLocation(handle, name);
}


DirectionalLighting::DirectionalLighting()
: program()
, light_direction(1.0f, 3.0f, 2.0f)
, light_color(0.9f, 0.9f, 0.9f)
, vp(1.0f)
, m_handle(0)
, mvp_handle(0)
, light_direction_handle(0)
, light_color_handle(0) {
	program.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"
		"layout(location = 0) in vec3 vtx_position;\n"
		"layout(location = 1) in vec3 vtx_color;\n"
		"layout(location = 2) in vec3 vtx_normal;\n"
		"uniform mat4 M;\n"
		"uniform mat4 MVP;\n"
		"out vec3 frag_color;\n"
		"out vec3 normal;\n"
		"void main() {\n"
			"gl_Position = MVP * vec4(vtx_position, 1);\n"
			"normal = (M * vec4(vtx_normal, 0)).xyz;\n"
			"frag_color = vtx_color;\n"
		"}\n"
	);
	program.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"
		"in vec3 frag_color;\n"
		"in vec3 normal;\n"
		"uniform vec3 light_direction;\n"
		"uniform vec3 light_color;\n"
		"out vec3 color;\n"
		"void main() {\n"
			"vec3 ambient = vec3(0.1, 0.1, 0.1) * frag_color;\n"
			"vec3 n = normalize(normal);\n"
			"vec3 l = normalize(light_direction);\n"
			"float cos_theta = clamp(dot(n, l), 0, 1);\n"
			"color = ambient + frag_color * light_color * cos_theta;\n"
		"}\n"
	);
	program.Link();
	if (!program.Linked()) {
		program.Log(std::cerr);
		throw std::runtime_error("link program");
	}

	m_handle = program.UniformLocation("M");
	mvp_handle = program.UniformLocation("MVP");
	light_direction_handle = program.UniformLocation("light_direction");
	light_color_handle = program.UniformLocation("light_color");
}


void DirectionalLighting::Activate() {
	GLContext::EnableDepthTest();
	GLContext::EnableBackfaceCulling();
	program.Use();

	glUniform3f(light_direction_handle, light_direction.x, light_direction.y, light_direction.z);
	glUniform3f(light_color_handle, light_color.x, light_color.y, light_color.z);
}

void DirectionalLighting::SetM(const glm::mat4 &m) {
	glm::mat4 mvp(vp * m);
	glUniformMatrix4fv(m_handle, 1, GL_FALSE, &m[0][0]);
	glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);
}

void DirectionalLighting::SetVP(const glm::mat4 &v, const glm::mat4 &p) {
	vp = p * v;
}

void DirectionalLighting::SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) {
	SetVP(v, p);
	SetM(m);
}

}
