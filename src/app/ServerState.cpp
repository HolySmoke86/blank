#include "ServerState.hpp"

#include "Environment.hpp"
#include "TextureIndex.hpp"
#include "../net/io.hpp"

#include <iostream>


namespace blank {

ServerState::ServerState(
	HeadlessEnvironment &env,
	const World::Config &wc,
	const WorldSave &ws,
	const Server::Config &sc
)
: env(env)
, block_types()
, world(block_types, wc, ws)
, server(sc, world) {
	TextureIndex tex_index;
	env.loader.LoadBlockTypes("default", block_types, tex_index);

	std::cout << "listening on UDP port " << sc.port << std::endl;
}


void ServerState::Handle(const SDL_Event &event) {
	if (event.type == SDL_QUIT) {
		env.state.PopAll();
	}
}


void ServerState::Update(int dt) {
	server.Handle();
	server.Update(dt);
}


void ServerState::Render(Viewport &viewport) {

}

}
