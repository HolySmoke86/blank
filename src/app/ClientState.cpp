#include "ClientState.hpp"

#include "Environment.hpp"
#include "init.hpp"
#include "TextureIndex.hpp"

namespace blank {

ClientState::ClientState(
	Environment &env,
	const World::Config &wc,
	const WorldSave &ws,
	const Interface::Config &ic,
	const Client::Config &cc
)
: env(env)
, block_types()
, world(block_types, wc, ws)
, chunk_renderer(world, wc.load.load_dist)
, interface(ic, env, world)
, client(cc, world) {
	TextureIndex tex_index;
	env.loader.LoadBlockTypes("default", block_types, tex_index);
	chunk_renderer.LoadTextures(env.loader, tex_index);
	chunk_renderer.FogDensity(wc.fog_density);
	// TODO: better solution for initializing HUD
	interface.SelectNext();
	client.SendLogin(ic.player_name);
}


void ClientState::OnEnter() {
	env.window.GrabMouse();
}


void ClientState::Handle(const SDL_Event &event) {
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
			env.state.Pop();
			break;
		default:
			break;
	}
}


void ClientState::Update(int dt) {
	client.Handle();
	client.Update(dt);
	if (client.TimedOut()) {
		env.state.Pop();
	}

	interface.Update(dt);
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


void ClientState::Render(Viewport &viewport) {
	viewport.WorldPosition(interface.Player().Transform(interface.Player().ChunkCoords()));
	chunk_renderer.Render(viewport);
	world.Render(viewport);
	interface.Render(viewport);
}

}
