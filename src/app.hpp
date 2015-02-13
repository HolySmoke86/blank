#ifndef BLANK_APP_HPP_
#define BLANK_APP_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.hpp"
#include "controller.hpp"
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
	FPSController modelCtrl;

	glm::vec3 light_position;
	glm::vec3 light_color;
	float light_power;

	GLuint m_handle;
	GLuint v_handle;
	GLuint mv_handle;
	GLuint mvp_handle;
	GLuint light_position_handle;
	GLuint light_color_handle;
	GLuint light_power_handle;

	bool running;

	bool front, back, left, right, up, down;

};

}

#endif
