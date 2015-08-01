#ifndef BLANK_APP_APPLICATION_HPP_
#define BLANK_APP_APPLICATION_HPP_

#include "Assets.hpp"
#include "FrameCounter.hpp"
#include "init.hpp"
#include "RandomWalk.hpp"
#include "../audio/Audio.hpp"
#include "../graphics/Viewport.hpp"
#include "../ui/Interface.hpp"
#include "../world/World.hpp"

#include <SDL.h>


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
	~Application();

	Application(const Application &) = delete;
	Application &operator =(const Application &) = delete;

	/// run until user quits
	void Run();
	/// evaluate a single frame of dt milliseconds
	void Loop(int dt);

	/// run for n frames
	void RunN(size_t n);
	/// run for t milliseconds
	void RunT(size_t t);
	/// run for n frames, assuming t milliseconds for each
	void RunS(size_t n, size_t t);

	/// process all events in SDL's queue
	void HandleEvents();
	void Handle(const SDL_WindowEvent &);
	/// integrate to the next step with dt milliseconds passed
	void Update(int dt);
	/// push the current state to display
	void Render();

	static Entity &MakeTestEntity(World &);

private:
	Init init;
	Viewport viewport;
	Assets assets;
	Audio audio;
	FrameCounter counter;

	World world;
	Interface interface;

	RandomWalk test_controller;

	bool running;

};

}

#endif
