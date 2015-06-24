#include "Application.hpp"

#include "../world/BlockType.hpp"
#include "../world/Entity.hpp"

#include <iostream>
#include <stdexcept>


namespace blank {

Application::Application(const Config &config)
: init_sdl()
, init_img()
, init_gl(config.doublebuf, config.multisampling)
, window()
, ctx(window.CreateContext())
, init_glew()
, chunk_prog()
, entity_prog()
, cam()
, world(config.world)
, interface(config.interface, world)
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
	HandleEvents();
	Update(dt);
	Render();
}


void Application::HandleEvents() {
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
				switch (event.window.event) {
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						window.GrabMouse();
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST:
						window.ReleaseMouse();
						break;
					case SDL_WINDOWEVENT_RESIZED:
						cam.Viewport(event.window.data1, event.window.data2);
						interface.Handle(event.window);
						break;
					default:
						interface.Handle(event.window);
						break;
				}
				break;
			default:
				break;
		}
	}
}

void Application::Update(int dt) {
	interface.Update(dt);
	test_controller.Update(dt);
	world.Update(dt);
}

void Application::Render() {
	GLContext::Clear();

	chunk_prog.SetProjection(cam.Projection());
	entity_prog.SetProjection(cam.Projection());

	world.Render(chunk_prog, entity_prog);

	interface.Render(entity_prog);

	window.Flip();
}

}
