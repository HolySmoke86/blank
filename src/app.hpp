#ifndef BLANK_APP_HPP_
#define BLANK_APP_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.hpp"
#include "init.hpp"
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
	void Render();

private:
	InitSDL init_sdl;
	InitIMG init_img;
	InitGL init_gl;
	Window window;
	GLContext ctx;
	InitGLEW init_glew;
	Program program;

	Camera cam;

	GLuint vtx_buf;
	GLuint mvp_handle;

	bool running;

};

}

#endif
