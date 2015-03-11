#include "app.hpp"

#include "geometry.hpp"

#include <iostream>
#include <stdexcept>


namespace blank {

Application::Application()
: init_sdl()
, init_img()
, init_gl()
, window()
, ctx(window.CreateContext())
, init_glew()
, program()
, cam()
, hud()
, world()
, controller(world.Player())
, outline()
, outline_visible(false)
, outline_transform(1.0f)
, running(false)
, place(false)
, remove(false)
, pick(false)
, remove_id(0)
, place_id(1) {
	GLContext::EnableVSync();

	world.Generate({ -4, -4, -4 }, { 5, 5, 5});

	hud.Viewport(960, 600);
	hud.Display(*world.BlockTypes()[place_id]);

	glClearColor(0.0, 0.0, 0.0, 1.0);
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
				controller.HandleKeyboard(event.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == 1) {
					// left
					remove = true;
				} else if (event.button.button == 2) {
					// middle
					pick = true;
				} else if (event.button.button == 3) {
					// right
					place = true;
				}
				break;
			case SDL_MOUSEMOTION:
				controller.HandleMouse(event.motion);
				break;
			case SDL_QUIT:
				running = false;
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						cam.Viewport(event.window.data1, event.window.data2);
						hud.Viewport(event.window.data1, event.window.data2);
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
}

void Application::Update(int dt) {
	controller.Update(dt);
	world.Update(dt);

	Ray aim = controller.Aim();
	Chunk *chunk;
	int blkid;
	float dist;
	glm::vec3 normal;
	if (world.Intersection(aim, glm::mat4(1.0f), &chunk, &blkid, &dist, &normal)) {
		glm::vec3 pos = Chunk::ToCoords(blkid);
		outline_visible = true;
		outline.Clear();
		chunk->Type(chunk->BlockAt(blkid)).FillOutlineModel(outline);
		outline_transform = glm::translate(chunk->Transform(world.Player().ChunkCoords()), pos);
		outline_transform = glm::scale(outline_transform, glm::vec3(1.0001f));
	} else {
		outline_visible = false;
	}

	if (pick) {
		if (chunk) {
			place_id = chunk->BlockAt(blkid).type;
			hud.Display(*world.BlockTypes()[place_id]);
		}
		pick = false;
	}
	if (remove) {
		if (chunk) {
			chunk->BlockAt(blkid).type = remove_id;
			chunk->Invalidate();
		}
		remove = false;
	}
	if (place) {
		if (chunk) {
			Chunk *mod_chunk = chunk;
			glm::vec3 next_pos = Chunk::ToCoords(blkid) + normal;
			if (!Chunk::InBounds(next_pos)) {
				mod_chunk = &world.Next(*chunk, normal);
				next_pos -= normal * glm::vec3(Chunk::Extent());
			}
			mod_chunk->BlockAt(next_pos).type = place_id;
			mod_chunk->Invalidate();
		}
		place = false;
	}
}

void Application::Render() {
	GLContext::Clear();

	program.Activate();

	program.SetProjection(cam.Projection());
	world.Render(program);

	if (outline_visible) {
		program.SetM(outline_transform);
		outline.Draw();
	}

	hud.Render(program);

	window.Flip();
}

}
