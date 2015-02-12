#include "app.hpp"

#include <iostream>
#include <stdexcept>


namespace {

constexpr GLfloat vtx_coords[] = {
	-1.0f, -1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	 0.0f,  1.0f, 0.0f,
};

}

namespace blank {

Application::Application()
: init_sdl()
, init_img()
, init_gl()
, window()
, ctx(window.CreateContext())
, init_glew()
, program()
, move_velocity(0.001f)
, pitch_sensitivity(-0.0025f)
, yaw_sensitivity(-0.001f)
, cam()
, model()
, vtx_buf(0)
, mvp_handle(0)
, running(false)
, front(false)
, back(false)
, left(false)
, right(false)
, up(false)
, down(false) {
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

	model.Position(glm::vec3(0, 0, -4));
	cam.Position(glm::vec3(0, 0, 4));

	mvp_handle = program.UniformLocation("MVP");

	glClearColor(0.0, 0.0, 0.0, 1.0);
}

Application::~Application() {

}


void Application::Run() {
	running = true;
	Uint32 last = SDL_GetTicks();
	window.GrabMouse();
	while (running) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void Application::Loop(int dt) {
	HandleEvents();
	Update(dt);
	Render();
}


void Application::HandleEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
					case SDLK_w:
						front = event.key.state == SDL_PRESSED;
						break;
					case SDLK_s:
						back = event.key.state == SDL_PRESSED;
						break;
					case SDLK_a:
						left = event.key.state == SDL_PRESSED;
						break;
					case SDLK_d:
						right = event.key.state == SDL_PRESSED;
						break;
					case SDLK_q:
						up = event.key.state == SDL_PRESSED;
						break;
					case SDLK_e:
						down = event.key.state == SDL_PRESSED;
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				cam.RotateYaw(event.motion.xrel * yaw_sensitivity);
				cam.RotatePitch(event.motion.yrel * pitch_sensitivity);
				break;
			case SDL_QUIT:
				running = false;
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						cam.Viewport(event.window.data1, event.window.data2);
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
}

void Application::Update(int dt) {
	glm::vec3 vel;
	if (right && !left) {
		vel.x = move_velocity;
	} else if (left && !right) {
		vel.x = -move_velocity;
	}
	if (up && !down) {
		vel.y = move_velocity;
	} else if (down && !up) {
		vel.y = -move_velocity;
	}
	if (back && !front) {
		vel.z = move_velocity;
	} else if (front && !back) {
		vel.z = -move_velocity;
	}
	cam.Velocity(vel);

	cam.Update(dt);
	model.Update(dt);
}

void Application::Render() {
	glClear(GL_COLOR_BUFFER_BIT);

	program.Use();

	glm::mat4 mvp(cam.MakeMVP(model.Transform()));
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
