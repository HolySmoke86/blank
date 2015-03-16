#ifndef BLANK_APP_HPP_
#define BLANK_APP_HPP_

#include "camera.hpp"
#include "controller.hpp"
#include "init.hpp"
#include "interface.hpp"
#include "shader.hpp"
#include "world.hpp"


namespace blank {

class Application {

public:
	Application();

	Application(const Application &) = delete;
	Application &operator =(const Application &) = delete;

	/// run until user quits
	void Run();
	void Loop(int dt);

	/// run for n frames
	void RunN(size_t n);
	/// run for t milliseconds
	void RunT(size_t t);
	/// run for n frames, assuming t milliseconds for each
	void RunS(size_t n, size_t t);

	void HandleEvents();
	void Update(int dt);
	void Render();

	static Entity &MakeTestEntity(World &);

private:
	InitSDL init_sdl;
	InitIMG init_img;
	InitGL init_gl;
	Window window;
	GLContext ctx;
	InitGLEW init_glew;
	DirectionalLighting program;

	Camera cam;
	World world;
	Interface interface;

	RandomWalk test_controller;

	bool running;

};

}

#endif
