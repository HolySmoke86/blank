#include "ClientState.hpp"

#include "Environment.hpp"
#include "TextureIndex.hpp"

namespace blank {

ClientState::ClientState(
	Environment &env,
	const World::Config &wc,
	const WorldSave &ws,
	const Client::Config &cc
)
: env(env)
, block_types()
, world(block_types, wc, ws)
, client(cc, world) {

}


void ClientState::Handle(const SDL_Event &event) {
	if (event.type == SDL_QUIT) {
		env.state.PopAll();
	}
}


void ClientState::Update(int dt) {
	client.Handle();
	client.Update(dt);
	if (client.TimedOut()) {
		env.state.Pop();
	}
}


void ClientState::Render(Viewport &viewport) {

}

}
