#ifndef BLANK_APP_HPP_
#define BLANK_APP_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.hpp"
#include "controller.hpp"
#include "hud.hpp"
#include "init.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "world.hpp"


namespace blank {

class Application {

public:
	Application();

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
	DirectionalLighting program;

	Camera cam;
	HUD hud;
	World world;

	OutlineModel outline;
	bool outline_visible;
	glm::mat4 outline_transform;

	bool running;

	bool place, remove, pick;

	int remove_id;
	int place_id;

};

}

#endif
