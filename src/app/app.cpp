#include "Application.hpp"
#include "Assets.hpp"
#include "FrameCounter.hpp"

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
: init_sdl()
, init_img()
, init_ttf()
, init_gl(config.doublebuf, config.multisampling)
, window()
, ctx(window.CreateContext())
, init_glew()
, assets(get_asset_path())
, counter()
, chunk_prog()
, entity_prog()
, sprite_prog()
, cam()
, world(config.world)
, interface(config.interface, assets, world)
, test_controller(MakeTestEntity(world))
, running(false) {
	if (config.vsync) {
		GLContext::EnableVSync();
	}

	glClearColor(0.0, 0.0, 0.0, 1.0);
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
	window.GrabMouse();
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
			window.GrabMouse();
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			window.ReleaseMouse();
			break;
		case SDL_WINDOWEVENT_RESIZED:
			cam.Viewport(event.data1, event.data2);
			interface.Handle(event);
			break;
		default:
			interface.Handle(event);
			break;
	}
}

void Application::Update(int dt) {
	counter.EnterUpdate();
	interface.Update(dt);
	test_controller.Update(dt);
	world.Update(dt);
	counter.ExitUpdate();
}

void Application::Render() {
	// gl implementation may (and will probably) delay vsync blocking until
	// the first write after flipping, which is this clear call
	GLContext::Clear();
	counter.EnterRender();

	chunk_prog.SetProjection(cam.Projection());
	entity_prog.SetProjection(cam.Projection());

	world.Render(chunk_prog, entity_prog);

	interface.Render(entity_prog, sprite_prog);

	counter.ExitRender();
	window.Flip();
}


Assets::Assets(const string &base)
: fonts(base + "fonts/") {

}

Font Assets::LoadFont(const string &name, int size) const {
	string full = fonts + name + ".ttf";
	return Font(full.c_str(), size);
}


void FrameCounter::EnterFrame() noexcept {
	last_enter = SDL_GetTicks();
	last_tick = last_enter;
}

void FrameCounter::EnterHandle() noexcept {
	Tick();
}

void FrameCounter::ExitHandle() noexcept {
	running.handle += Tick();
}

void FrameCounter::EnterUpdate() noexcept {
	Tick();
}

void FrameCounter::ExitUpdate() noexcept {
	running.update += Tick();
}

void FrameCounter::EnterRender() noexcept {
	Tick();
}

void FrameCounter::ExitRender() noexcept {
	running.render += Tick();
}

void FrameCounter::ExitFrame() noexcept {
	Uint32 now = SDL_GetTicks();
	running.total += now - last_enter;
	++cur_frame;
	if (cur_frame >= NUM_FRAMES) {
		avg.handle = running.handle * factor;
		avg.update = running.update * factor;
		avg.render = running.render * factor;
		avg.total = running.total * factor;
		running = Frame<int>{};
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

void FrameCounter::Print(std::ostream &out) const {
	out << "frame:     " << AvgFrame() << std::endl;
	out << "  handle:  " << AvgHandle() << std::endl;
	out << "  update:  " << AvgUpdate() << std::endl;
	out << "  render:  " << AvgRender() << std::endl;
	out << "  running: " << AvgRunning() << std::endl;
	out << "  waiting: " << AvgWaiting() << std::endl;
	out << std::endl;
}

}
