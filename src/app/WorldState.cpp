#include "WorldState.hpp"

#include "Environment.hpp"
#include "init.hpp"
#include "TextureIndex.hpp"

#include <SDL.h>


namespace blank {

WorldState::WorldState(
	Environment &env,
	const Interface::Config &ic,
	const World::Config &wc,
	const WorldSave &save
)
: env(env)
, block_types()
, world(block_types, wc, save)
, chunk_renderer(world, wc.load.load_dist)
, spawner(world, wc.gen.seed)
, interface(ic, env, world)
, preload(env, world.Loader(), chunk_renderer)
, unload(env, world.Loader()) {
	TextureIndex tex_index;
	env.loader.LoadBlockTypes("default", block_types, tex_index);
	chunk_renderer.LoadTextures(env.loader, tex_index);
	chunk_renderer.FogDensity(wc.fog_density);
	// TODO: better solution for initializing HUD
	interface.SelectNext();
}


void WorldState::OnEnter() {
	env.state.Push(&preload);
	env.window.GrabMouse();
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
		case SDL_QUIT:
			env.state.Switch(&unload);
			break;
		default:
			break;
	}
}

void WorldState::Update(int dt) {
	interface.Update(dt);
	spawner.Update(dt);
	world.Update(dt);
	chunk_renderer.Rebase(interface.Player().ChunkCoords());
	chunk_renderer.Update(dt);

	glm::mat4 trans = interface.Player().Transform(interface.Player().ChunkCoords());
	glm::vec3 dir(trans * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	glm::vec3 up(trans * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	env.audio.Position(interface.Player().Position());
	env.audio.Velocity(interface.Player().Velocity());
	env.audio.Orientation(dir, up);
}

void WorldState::Render(Viewport &viewport) {
	viewport.WorldPosition(interface.Player().Transform(interface.Player().ChunkCoords()));
	chunk_renderer.Render(viewport);
	world.Render(viewport);
	interface.Render(viewport);
}

}
