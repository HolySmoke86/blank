#include "WorldState.hpp"

#include "Environment.hpp"

#include <SDL.h>


namespace blank {

WorldState::WorldState(
	Environment &env,
	const Interface::Config &ic,
	const World::Config &wc
)
: env(env)
, world(env.assets, wc)
, spawner(world)
, interface(ic, env, world) {

}


void WorldState::Handle(const SDL_Event &event) {
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
		default:
			break;
	}
}

void WorldState::Update(int dt) {
	interface.Update(dt);
	spawner.Update(dt);
	world.Update(dt);

	glm::mat4 trans = world.Player().Transform(Chunk::Pos(0, 0, 0));
	glm::vec3 dir(trans * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	glm::vec3 up(trans * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	env.audio.Position(world.Player().Position());
	env.audio.Velocity(world.Player().Velocity());
	env.audio.Orientation(dir, up);

}

void WorldState::Render(Viewport &viewport) {
	world.Render(viewport);
	interface.Render(viewport);
}

}
