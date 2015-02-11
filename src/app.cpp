#include "app.hpp"

#include <iostream>
#include <stdexcept>


namespace {

constexpr GLfloat vtx_coords[] = {
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 0.0f,  1.0f, -1.0f,
};

}

namespace blank {

Application::Application()
: init_sdl()
, init_gl()
, window()
, ctx(window.CreateContext())
, init_glew()
, program()
, vtx_buf(0)
, mvp()
, mvp_handle(0)
, running(false) {
	GLContext::EnableVSync();
	program.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"
		"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
		"uniform mat4 MVP;\n"
		"void main() {\n"
			"vec4 v = vec4(vertexPosition_modelspace, 1);\n"
			"gl_Position = MVP * v;\n"
		"}\n"
	);
	program.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"
		"out vec3 color;\n"
		"void main() {\n"
			"color = vec3(1, 1, 1);\n"
		"}\n"
	);
	program.Link();
	if (!program.Linked()) {
		program.Log(std::cerr);
		throw std::runtime_error("link program");
	}

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vtx_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vtx_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtx_coords), vtx_coords, GL_STATIC_DRAW);

	glm::mat4 projection = glm::perspective(
		45.0f, // FOV in degrees
		1.0f,  // aspect ratio
		0.1f,  // near clip
		100.0f // far clip
	);
	glm::mat4 view = glm::lookAt(
		glm::vec3(0, 0,  0),  // observer
		glm::vec3(0, 0, -1), // target
		glm::vec3(0, 1,  0) // up
	);
	glm::mat4 model(1.0f); // identity: no transformation
	mvp = projection * view * model;

	mvp_handle = program.UniformLocation("MVP");

	glClearColor(0.0, 0.0, 0.0, 1.0);
}

Application::~Application() {

}


void Application::Run() {
	running = true;
	Uint32 last = SDL_GetTicks();
	while (running) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void Application::Loop(int dt) {
	HandleEvents();
	Render();
}


void Application::HandleEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			default:
				break;
		}
	}
}

void Application::Render() {
	glClear(GL_COLOR_BUFFER_BIT);

	program.Use();

	glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vtx_buf);
	glVertexAttribPointer(
		0,        // attribute 0 (for shader)
		3,        // size
		GL_FLOAT, // type
		GL_FALSE, // normalized
		0,        // stride
		nullptr   // offset
	);
	glDrawArrays(
		GL_TRIANGLES, // how
		0,            // start
		3             // len
	);
	glDisableVertexAttribArray(0);

	window.Flip();
}

}
