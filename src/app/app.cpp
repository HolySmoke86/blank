#include "Application.hpp"
#include "Assets.hpp"
#include "FrameCounter.hpp"

#include "../audio/Sound.hpp"
#include "../graphics/Font.hpp"
#include "../world/BlockType.hpp"
#include "../world/Entity.hpp"

#include <iostream>
#include <stdexcept>

using std::string;


namespace {

string get_asset_path() {
	char *base = SDL_GetBasePath();
	string assets(base);
	assets += "assets/";
	SDL_free(base);
	return assets;
}

}

namespace blank {

Application::Application(const Config &config)
: init(config.doublebuf, config.multisampling)
, viewport()
, assets(get_asset_path())
, audio()
, counter()
, world(config.world)
, interface(config.interface, assets, audio, counter, world)
, test_controller(MakeTestEntity(world))
, running(false) {
	viewport.VSync(config.vsync);
}

Application::~Application() {
	audio.StopAll();
}

Entity &Application::MakeTestEntity(World &world) {
	Entity &e = world.AddEntity();
	e.Name("test");
	e.Position({ 0.0f, 0.0f, 0.0f });
	e.Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	e.WorldCollidable(true);
	e.SetShape(world.BlockTypes()[1].shape, { 1.0f, 1.0f, 0.0f });
	e.AngularVelocity(glm::quat(glm::vec3{ 0.00001f, 0.000006f, 0.000013f }));
	return e;
}


void Application::RunN(size_t n) {
	Uint32 last = SDL_GetTicks();
	for (size_t i = 0; i < n; ++i) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void Application::RunT(size_t t) {
	Uint32 last = SDL_GetTicks();
	Uint32 finish = last + t;
	while (last < finish) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void Application::RunS(size_t n, size_t t) {
	for (size_t i = 0; i < n; ++i) {
		Loop(t);
	}
}


void Application::Run() {
	running = true;
	Uint32 last = SDL_GetTicks();
	init.window.GrabMouse();
	while (running) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void Application::Loop(int dt) {
	counter.EnterFrame();
	HandleEvents();
	Update(dt);
	Render();
	counter.ExitFrame();
}


void Application::HandleEvents() {
	counter.EnterHandle();
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN:
				interface.HandlePress(event.key);
				break;
			case SDL_KEYUP:
				interface.HandleRelease(event.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
				interface.HandlePress(event.button);
				break;
			case SDL_MOUSEBUTTONUP:
				interface.HandleRelease(event.button);
				break;
			case SDL_MOUSEMOTION:
				interface.Handle(event.motion);
				break;
			case SDL_MOUSEWHEEL:
				interface.Handle(event.wheel);
				break;
			case SDL_QUIT:
				running = false;
				break;
			case SDL_WINDOWEVENT:
				Handle(event.window);
				break;
			default:
				break;
		}
	}
	counter.ExitHandle();
}

void Application::Handle(const SDL_WindowEvent &event) {
	switch (event.event) {
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			init.window.GrabMouse();
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			init.window.ReleaseMouse();
			break;
		case SDL_WINDOWEVENT_RESIZED:
			viewport.Resize(event.data1, event.data2);
			break;
		default:
			break;
	}
}

void Application::Update(int dt) {
	counter.EnterUpdate();
	interface.Update(dt);
	test_controller.Update(dt);
	world.Update(dt);

	glm::mat4 trans = world.Player().Transform(Chunk::Pos(0, 0, 0));
	glm::vec3 dir(trans * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	glm::vec3 up(trans * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	audio.Position(world.Player().Position());
	audio.Velocity(world.Player().Velocity());
	audio.Orientation(dir, up);

	counter.ExitUpdate();
}

void Application::Render() {
	// gl implementation may (and will probably) delay vsync blocking until
	// the first write after flipping, which is this clear call
	viewport.Clear();
	counter.EnterRender();

	world.Render(viewport);
	interface.Render(viewport);

	counter.ExitRender();
	init.window.Flip();
}


Assets::Assets(const string &base)
: fonts(base + "fonts/")
, sounds(base + "sounds/") {

}

Font Assets::LoadFont(const string &name, int size) const {
	string full = fonts + name + ".ttf";
	return Font(full.c_str(), size);
}

Sound Assets::LoadSound(const string &name) const {
	string full = sounds + name + ".wav";
	return Sound(full.c_str());
}


void FrameCounter::EnterFrame() noexcept {
	last_enter = SDL_GetTicks();
	last_tick = last_enter;
}

void FrameCounter::EnterHandle() noexcept {
	Tick();
}

void FrameCounter::ExitHandle() noexcept {
	current.handle = Tick();
}

void FrameCounter::EnterUpdate() noexcept {
	Tick();
}

void FrameCounter::ExitUpdate() noexcept {
	current.update = Tick();
}

void FrameCounter::EnterRender() noexcept {
	Tick();
}

void FrameCounter::ExitRender() noexcept {
	current.render = Tick();
}

void FrameCounter::ExitFrame() noexcept {
	Uint32 now = SDL_GetTicks();
	current.total = now - last_enter;
	current.running = current.handle + current.update + current.render;
	current.waiting = current.total - current.running;
	Accumulate();

	++cur_frame;
	if (cur_frame >= NUM_FRAMES) {
		Push();
		cur_frame = 0;
		changed = true;
	} else {
		changed = false;
	}
}

int FrameCounter::Tick() noexcept {
	Uint32 now = SDL_GetTicks();
	int delta = now - last_tick;
	last_tick = now;
	return delta;
}

void FrameCounter::Accumulate() noexcept {
	sum.handle += current.handle;
	sum.update += current.update;
	sum.render += current.render;
	sum.running += current.running;
	sum.waiting += current.waiting;
	sum.total += current.total;

	max.handle = std::max(current.handle, max.handle);
	max.update = std::max(current.update, max.update);
	max.render = std::max(current.render, max.render);
	max.running = std::max(current.running, max.running);
	max.waiting = std::max(current.waiting, max.waiting);
	max.total = std::max(current.total, max.total);

	current = Frame<int>();
}

void FrameCounter::Push() noexcept {
	peak = max;
	avg.handle = sum.handle * factor;
	avg.update = sum.update * factor;
	avg.render = sum.render * factor;
	avg.running = sum.running * factor;
	avg.waiting = sum.waiting * factor;
	avg.total = sum.total * factor;

	sum = Frame<int>();
	max = Frame<int>();
}

}
