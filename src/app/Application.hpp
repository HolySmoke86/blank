#ifndef BLANK_APP_APPLICATION_HPP_
#define BLANK_APP_APPLICATION_HPP_

#include "init.hpp"
#include "RandomWalk.hpp"
#include "../graphics/BlockLighting.hpp"
#include "../graphics/Camera.hpp"
#include "../graphics/DirectionalLighting.hpp"
#include "../ui/Interface.hpp"
#include "../world/World.hpp"


namespace blank {

class Application {

public:
	struct Config {
		bool vsync = true;
		bool doublebuf = true;
		int multisampling = 1;

		Interface::Config interface = Interface::Config();
		World::Config world = World::Config();
	};

	explicit Application(const Config &);

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
	BlockLighting chunk_prog;
	DirectionalLighting entity_prog;

	Camera cam;
	World world;
	Interface interface;

	RandomWalk test_controller;

	bool running;

};

}

#endif
