#include "BlendedSprite.hpp"
#include "BlockLighting.hpp"
#include "DirectionalLighting.hpp"
#include "Program.hpp"
#include "Shader.hpp"

#include "Texture.hpp"
#include "../app/init.hpp"

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

Shader::Shader(Shader &&other) noexcept
: handle(other.handle) {
	other.handle = 0;
}

Shader &Shader::operator =(Shader &&other) noexcept {
	std::swap(handle, other.handle);
	return *this;
}


void Shader::Source(const GLchar *src) noexcept {
	const GLchar* src_arr[] = { src };
	glShaderSource(handle, 1, src_arr, nullptr);
}

void Shader::Compile() noexcept {
	glCompileShader(handle);
}

bool Shader::Compiled() const noexcept {
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


void Shader::AttachToProgram(GLuint id) const noexcept {
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

void Program::Attach(Shader &shader) noexcept {
	shader.AttachToProgram(handle);
}

void Program::Link() noexcept {
	glLinkProgram(handle);
}

bool Program::Linked() const noexcept {
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


GLint Program::AttributeLocation(const GLchar *name) const noexcept {
	return glGetAttribLocation(handle, name);
}

GLint Program::UniformLocation(const GLchar *name) const noexcept {
	return glGetUniformLocation(handle, name);
}


DirectionalLighting::DirectionalLighting()
: program()
, light_direction(1.0f, 3.0f, 2.0f)
, light_color(0.9f, 0.9f, 0.9f)
, vp(1.0f)
, m_handle(0)
, mv_handle(0)
, mvp_handle(0)
, light_direction_handle(0)
, light_color_handle(0)
, fog_density_handle(0) {
	program.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"
		"layout(location = 0) in vec3 vtx_position;\n"
		"layout(location = 1) in vec3 vtx_color;\n"
		"layout(location = 2) in vec3 vtx_normal;\n"
		"uniform mat4 M;\n"
		"uniform mat4 MV;\n"
		"uniform mat4 MVP;\n"
		"out vec3 frag_color;\n"
		"out vec3 vtx_viewspace;\n"
		"out vec3 normal;\n"
		"void main() {\n"
			"gl_Position = MVP * vec4(vtx_position, 1);\n"
			"vtx_viewspace = (MV * vec4(vtx_position, 1)).xyz;\n"
			"normal = (M * vec4(vtx_normal, 0)).xyz;\n"
			"frag_color = vtx_color;\n"
		"}\n"
	);
	program.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"
		"in vec3 frag_color;\n"
		"in vec3 vtx_viewspace;\n"
		"in vec3 normal;\n"
		"uniform vec3 light_direction;\n"
		"uniform vec3 light_color;\n"
		"uniform float fog_density;\n"
		"out vec3 color;\n"
		"void main() {\n"
			"vec3 ambient = vec3(0.1, 0.1, 0.1) * frag_color;\n"
			// this should be the same as the clear color, otherwise looks really weird
			"vec3 fog_color = vec3(0, 0, 0);\n"
			"float e = 2.718281828;\n"
			"vec3 n = normalize(normal);\n"
			"vec3 l = normalize(light_direction);\n"
			"float cos_theta = clamp(dot(n, l), 0, 1);\n"
			"vec3 reflect_color = ambient + frag_color * light_color * cos_theta;\n"
			"float value = pow(e, -pow(fog_density * length(vtx_viewspace), 5));"
			"color = mix(fog_color, reflect_color, value);\n"
		"}\n"
	);
	program.Link();
	if (!program.Linked()) {
		program.Log(std::cerr);
		throw std::runtime_error("link program");
	}

	m_handle = program.UniformLocation("M");
	mv_handle = program.UniformLocation("MV");
	mvp_handle = program.UniformLocation("MVP");
	light_direction_handle = program.UniformLocation("light_direction");
	light_color_handle = program.UniformLocation("light_color");
	fog_density_handle = program.UniformLocation("fog_density");
}


void DirectionalLighting::Activate() noexcept {
	GLContext::EnableDepthTest();
	GLContext::EnableBackfaceCulling();
	program.Use();

	glUniform3f(light_direction_handle, light_direction.x, light_direction.y, light_direction.z);
	glUniform3f(light_color_handle, light_color.x, light_color.y, light_color.z);
}

void DirectionalLighting::SetM(const glm::mat4 &m) noexcept {
	glm::mat4 mv(view * m);
	glm::mat4 mvp(vp * m);
	glUniformMatrix4fv(m_handle, 1, GL_FALSE, &m[0][0]);
	glUniformMatrix4fv(mv_handle, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);
}

void DirectionalLighting::SetLightDirection(const glm::vec3 &dir) noexcept {
	light_direction = -dir;
	glUniform3f(light_direction_handle, light_direction.x, light_direction.y, light_direction.z);
}

void DirectionalLighting::SetFogDensity(float f) noexcept {
	fog_density = f;
	glUniform1f(fog_density_handle, fog_density);
}

void DirectionalLighting::SetProjection(const glm::mat4 &p) noexcept {
	projection = p;
	vp = p * view;
}

void DirectionalLighting::SetView(const glm::mat4 &v) noexcept {
	view = v;
	vp = projection * v;
}

void DirectionalLighting::SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept {
	projection = p;
	view = v;
	vp = p * v;
}

void DirectionalLighting::SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept {
	SetVP(v, p);
	SetM(m);
}


BlockLighting::BlockLighting()
: program()
, vp(1.0f)
, mv_handle(0)
, mvp_handle(0)
, fog_density_handle(0) {
	program.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"
		"layout(location = 0) in vec3 vtx_position;\n"
		"layout(location = 1) in vec3 vtx_color;\n"
		"layout(location = 2) in float vtx_light;\n"
		"uniform mat4 MV;\n"
		"uniform mat4 MVP;\n"
		"out vec3 frag_color;\n"
		"out vec3 vtx_viewspace;\n"
		"out float frag_light;\n"
		"void main() {\n"
			"gl_Position = MVP * vec4(vtx_position, 1);\n"
			"frag_color = vtx_color;\n"
			"vtx_viewspace = (MV * vec4(vtx_position, 1)).xyz;\n"
			"frag_light = vtx_light;\n"
		"}\n"
	);
	program.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"
		"in vec3 frag_color;\n"
		"in vec3 vtx_viewspace;\n"
		"in float frag_light;\n"
		"uniform float fog_density;\n"
		"out vec3 color;\n"
		"void main() {\n"
			"vec3 ambient = vec3(0.1, 0.1, 0.1) * frag_color;\n"
			"float light_power = clamp(pow(0.8, 15 - frag_light), 0, 1);\n"
			"vec3 fog_color = vec3(0, 0, 0);\n"
			"float e = 2.718281828;\n"
			//"vec3 reflect_color = ambient + frag_color * light_power;\n"
			"vec3 reflect_color = frag_color * light_power;\n"
			"float value = pow(e, -pow(fog_density * length(vtx_viewspace), 5));"
			"color = mix(fog_color, reflect_color, value);\n"
		"}\n"
	);
	program.Link();
	if (!program.Linked()) {
		program.Log(std::cerr);
		throw std::runtime_error("link program");
	}

	mv_handle = program.UniformLocation("MV");
	mvp_handle = program.UniformLocation("MVP");
	fog_density_handle = program.UniformLocation("fog_density");
}


void BlockLighting::Activate() noexcept {
	GLContext::EnableDepthTest();
	GLContext::EnableBackfaceCulling();
	GLContext::DisableAlphaBlending();
	program.Use();
}

void BlockLighting::SetM(const glm::mat4 &m) noexcept {
	glm::mat4 mv(view * m);
	glm::mat4 mvp(vp * m);
	glUniformMatrix4fv(mv_handle, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);
}

void BlockLighting::SetFogDensity(float f) noexcept {
	fog_density = f;
	glUniform1f(fog_density_handle, fog_density);
}

void BlockLighting::SetProjection(const glm::mat4 &p) noexcept {
	projection = p;
	vp = p * view;
}

void BlockLighting::SetView(const glm::mat4 &v) noexcept {
	view = v;
	vp = projection * v;
}

void BlockLighting::SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept {
	projection = p;
	view = v;
	vp = p * v;
}

void BlockLighting::SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept {
	SetVP(v, p);
	SetM(m);
}


BlendedSprite::BlendedSprite()
: program()
, vp(1.0f)
, mvp_handle(0)
, sampler_handle(0) {
	program.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"
		"layout(location = 0) in vec3 vtx_position;\n"
		"layout(location = 1) in vec2 vtx_tex_uv;\n"
		"uniform mat4 MVP;\n"
		"out vec2 frag_tex_uv;\n"
		"void main() {\n"
			"gl_Position = MVP * vec4(vtx_position, 1);\n"
			"frag_tex_uv = vtx_tex_uv;\n"
		"}\n"
	);
	program.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"
		"in vec2 frag_tex_uv;\n"
		"uniform sampler2D tex_sampler;\n"
		"out vec4 color;\n"
		"void main() {\n"
			"color = texture(tex_sampler, frag_tex_uv);\n"
		"}\n"
	);
	program.Link();
	if (!program.Linked()) {
		program.Log(std::cerr);
		throw std::runtime_error("link program");
	}

	mvp_handle = program.UniformLocation("MVP");
	sampler_handle = program.UniformLocation("tex_sampler");
}


void BlendedSprite::Activate() noexcept {
	GLContext::EnableAlphaBlending();
	program.Use();
}

void BlendedSprite::SetM(const glm::mat4 &m) noexcept {
	glm::mat4 mvp(vp * m);
	glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);
}

void BlendedSprite::SetProjection(const glm::mat4 &p) noexcept {
	projection = p;
	vp = p * view;
}

void BlendedSprite::SetView(const glm::mat4 &v) noexcept {
	view = v;
	vp = projection * v;
}

void BlendedSprite::SetVP(const glm::mat4 &v, const glm::mat4 &p) noexcept {
	projection = p;
	view = v;
	vp = p * v;
}

void BlendedSprite::SetMVP(const glm::mat4 &m, const glm::mat4 &v, const glm::mat4 &p) noexcept {
	SetVP(v, p);
	SetM(m);
}

void BlendedSprite::SetTexture(Texture &tex) noexcept {
	glActiveTexture(GL_TEXTURE0);
	tex.Bind();
	glUniform1i(sampler_handle, 0);
}

}
