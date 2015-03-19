#include "app.hpp"

#include <iostream>
#include <stdexcept>


namespace blank {

Application::Application(unsigned int seed)
: init_sdl()
, init_img()
, init_gl()
, window()
, ctx(window.CreateContext())
, init_glew()
, program()
, cam()
, world(seed)
, interface(world)
, test_controller(MakeTestEntity(world))
, running(false) {
	GLContext::EnableVSync();

	glClearColor(0.0, 0.0, 0.0, 1.0);
}

Entity &Application::MakeTestEntity(World &world) {
	Entity &e = world.AddEntity();
	e.Position({ 0.0f, 0.0f, 0.0f });
	e.SetShape(world.BlockTypes()[1]->shape, { 1.0f, 1.0f, 0.0f });
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
			case SDL_KEYUP:
				interface.Handle(event.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
				interface.Handle(event.button);
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

	program.Activate();

	program.SetProjection(cam.Projection());
	world.Render(program);

	interface.Render(program);

	window.Flip();
}

}
