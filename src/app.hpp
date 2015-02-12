#ifndef BLANK_APP_HPP_
#define BLANK_APP_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.hpp"
#include "init.hpp"
#include "model.hpp"
#include "shader.hpp"


namespace blank {

class Application {

public:
	Application();
	~Application();

	Application(const Application &) = delete;
	Application &operator =(const Application &) = delete;

	void Run();
	void Loop(int dt);

	void HandleEvents();
	void Update(int dt);
	void Render();

private:
	InitSDL init_sdl;
	InitIMG init_img;
	InitGL init_gl;
	Window window;
	GLContext ctx;
	InitGLEW init_glew;
	Program program;

	float move_velocity;
	float pitch_sensitivity;
	float yaw_sensitivity;

	Camera cam;
	Model model;

	GLuint vtx_buf;
	GLuint mvp_handle;

	bool running;

	bool front, back, left, right, up, down;

};

}

#endif
