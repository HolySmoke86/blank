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
, move_velocity(0.003f)
, pitch_sensitivity(-0.0025f)
, yaw_sensitivity(-0.001f)
, cam()
, world()
, outline()
, outline_visible(false)
, outline_transform(1.0f)
, running(false)
, front(false)
, back(false)
, left(false)
, right(false)
, up(false)
, down(false)
, place(false)
, remove(false)
, pick(false)
, remove_id(0)
, place_id(1) {
	GLContext::EnableVSync();

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	cam.Position(glm::vec3(0, 4, 4));

	world.Generate();

	glClearColor(0.0, 0.0, 0.0, 1.0);
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
				switch (event.key.keysym.sym) {
					case SDLK_w:
						front = event.key.state == SDL_PRESSED;
						break;
					case SDLK_s:
						back = event.key.state == SDL_PRESSED;
						break;
					case SDLK_a:
						left = event.key.state == SDL_PRESSED;
						break;
					case SDLK_d:
						right = event.key.state == SDL_PRESSED;
						break;
					case SDLK_q:
					case SDLK_SPACE:
						up = event.key.state == SDL_PRESSED;
						break;
					case SDLK_e:
					case SDLK_LSHIFT:
						down = event.key.state == SDL_PRESSED;
						break;
				}
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
				cam.RotateYaw(event.motion.xrel * yaw_sensitivity);
				cam.RotatePitch(event.motion.yrel * pitch_sensitivity);
				break;
			case SDL_QUIT:
				running = false;
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						cam.Viewport(event.window.data1, event.window.data2);
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
	glm::vec3 vel;
	if (right && !left) {
		vel.x = move_velocity;
	} else if (left && !right) {
		vel.x = -move_velocity;
	}
	if (up && !down) {
		vel.y = move_velocity;
	} else if (down && !up) {
		vel.y = -move_velocity;
	}
	if (back && !front) {
		vel.z = move_velocity;
	} else if (front && !back) {
		vel.z = -move_velocity;
	}
	cam.OrientationVelocity(vel);

	cam.Update(dt);

	Ray aim = cam.Aim();
	Chunk *chunk;
	int blkid;
	float dist;
	glm::vec3 normal;
	if (world.Intersection(aim, glm::mat4(1.0f), &chunk, &blkid, &dist, &normal)) {
		glm::vec3 pos = Chunk::ToCoords(blkid);
		outline_visible = true;
		outline.Clear();
		chunk->BlockAt(blkid).type->FillOutlineModel(outline);
		outline_transform = glm::translate(chunk->Transform(), pos);
	} else {
		outline_visible = false;
	}

	if (pick) {
		if (chunk) {
			place_id = chunk->BlockAt(blkid).type->id;
		}
		pick = false;
	}
	if (remove) {
		if (chunk) {
			chunk->BlockAt(blkid).type = world.BlockTypes()[remove_id];
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
				next_pos -= normal * Chunk::Extent();
			}
			mod_chunk->BlockAt(next_pos).type = world.BlockTypes()[place_id];
			mod_chunk->Invalidate();
		}
		place = false;
	}
}

void Application::Render() {
	GLContext::Clear();

	program.Activate();

	program.SetVP(cam.View(), cam.Projection());

	for (Chunk &chunk : world.LoadedChunks()) {
		program.SetM(chunk.Transform());
		chunk.Draw();
	}

	if (outline_visible) {
		program.SetM(outline_transform);
		outline.Draw();
	}

	window.Flip();
}

}
