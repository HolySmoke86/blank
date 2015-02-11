#include "shader.hpp"

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

}
