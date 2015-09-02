#ifndef BLANK_APP_CLIENTSTATE_HPP_
#define BLANK_APP_CLIENTSTATE_HPP_

#include "State.hpp"
#include "../net/Client.hpp"
#include "../ui/Interface.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/ChunkRenderer.hpp"
#include "../world/World.hpp"


namespace blank {

class Environment;

class ClientState
: public State {

public:
	ClientState(
		Environment &,
		const World::Config &,
		const WorldSave &,
		const Interface::Config &,
		const Client::Config &
	);

	void OnEnter() override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

private:
	Environment &env;
	BlockTypeRegistry block_types;
	World world;
	ChunkRenderer chunk_renderer;
	Interface interface;
	Client client;

};

}

#endif
