#include "WorldState.hpp"

#include "Environment.hpp"
#include "init.hpp"
#include "TextureIndex.hpp"

#include <SDL.h>


namespace blank {

WorldState::WorldState(
	Environment &env,
	const Generator::Config &gc,
	const Interface::Config &ic,
	const World::Config &wc,
	const WorldSave &save
)
: env(env)
, block_types()
, world(block_types, wc)
, interface(ic, env, world, world.AddPlayer(ic.player_name))
, generator(gc)
, chunk_loader(world.Chunks(), generator, save)
, chunk_renderer(*interface.GetPlayer().chunks)
, skeletons()
, spawner(world, skeletons, gc.seed)
, sky(env.loader.LoadCubeMap("skybox"))
, preload(env, chunk_loader, chunk_renderer)
, unload(env, world.Chunks(), save) {
	TextureIndex tex_index;
	env.loader.LoadBlockTypes("default", block_types, tex_index);
	chunk_renderer.LoadTextures(env.loader, tex_index);
	chunk_renderer.FogDensity(wc.fog_density);
	skeletons.Load();
	spawner.LimitSkeletons(0, skeletons.Size());
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
	chunk_loader.Update(dt);
	chunk_renderer.Update(dt);

	Entity &player = *interface.GetPlayer().entity;

	glm::mat4 trans = player.Transform(player.ChunkCoords());
	glm::vec3 dir(trans * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	glm::vec3 up(trans * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	env.audio.Position(player.Position());
	env.audio.Velocity(player.Velocity());
	env.audio.Orientation(dir, up);
}

void WorldState::Render(Viewport &viewport) {
	Entity &player = *interface.GetPlayer().entity;
	viewport.WorldPosition(player.Transform(player.ChunkCoords()));
	chunk_renderer.Render(viewport);
	world.Render(viewport);
	sky.Render(viewport);

	interface.Render(viewport);
}

}
