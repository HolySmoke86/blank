#ifndef BLANK_APP_APPLICATION_HPP_
#define BLANK_APP_APPLICATION_HPP_

#include "Assets.hpp"
#include "FrameCounter.hpp"
#include "../ai/Spawner.hpp"
#include "../audio/Audio.hpp"
#include "../graphics/Viewport.hpp"
#include "../ui/Interface.hpp"
#include "../world/World.hpp"

#include <SDL.h>


namespace blank {

class Window;

class Application {

public:
	struct Config {
		bool vsync = true;
		bool doublebuf = true;
		int multisampling = 1;

		Interface::Config interface = Interface::Config();
		World::Config world = World::Config();
	};

	Application(Window &, const Config &);
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

private:
	Window &window;
	Viewport viewport;
	Assets assets;
	Audio audio;
	FrameCounter counter;

	World world;
	Interface interface;

	Spawner spawner;

	bool running;

};

}

#endif
