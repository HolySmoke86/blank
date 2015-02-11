#include <iostream>
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "init.hpp"
#include "shader.hpp"

using namespace std;
using namespace blank;


constexpr GLfloat vtx_coords[] = {
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 0.0f,  1.0f, -1.0f,
};


int main(int argc, char *argv[]) {

	InitSDL init_sdl;
	InitGL init_gl;
	Window window;

	GLContext ctx = window.CreateContext();
	InitGLEW init_glew;
	GLContext::EnableVSync();


	Shader vtx_shader(GL_VERTEX_SHADER);
	vtx_shader.Source(
		"#version 330 core\n"
		"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
		"uniform mat4 MVP;\n"
		"void main() {\n"
			"vec4 v = vec4(vertexPosition_modelspace, 1);\n"
			"gl_Position = MVP * v;\n"
		"}\n"
	);
	vtx_shader.Compile();

	if (!vtx_shader.Compiled()) {
		cerr << "vertex shader compile error" << endl;
		vtx_shader.Log(cerr);
		return 4;
	}

	Shader frag_shader(GL_FRAGMENT_SHADER);
	frag_shader.Source(
		"#version 330 core\n"
		"out vec3 color;\n"
		"void main() {\n"
			"color = vec3(1, 1, 1);\n"
		"}\n"
	);
	frag_shader.Compile();

	if (!frag_shader.Compiled()) {
		cerr << "fragment shader compile error" << endl;
		frag_shader.Log(cerr);
		return 4;
	}


	Program program;
	program.Attach(vtx_shader);
	program.Attach(frag_shader);
	program.Link();

	if (!program.Linked()) {
		cerr << "program link error" << endl;
		program.Log(cerr);
		return 4;
	}


	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);


	GLuint vtx_buf;
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
	glm::mat4 mvp = projection * view * model;

	GLuint mvp_id = program.UniformLocation("MVP");


	glClearColor(0.0, 0.0, 0.0, 1.0);


	bool running = true;
	Uint32 last = SDL_GetTicks();
	while (running) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;

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

		glClear(GL_COLOR_BUFFER_BIT);

		program.Use();

		glUniformMatrix4fv(mvp_id, 1, GL_FALSE, &mvp[0][0]);

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

		last = now;
	}

	return 0;

}
