#ifndef BLANK_APP_SERVERSTATE_HPP_
#define BLANK_APP_SERVERSTATE_HPP_

#include "State.hpp"
#include "../net/Server.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/World.hpp"


namespace blank {

class HeadlessEnvironment;

class ServerState
: public State {

public:
	ServerState(
		HeadlessEnvironment &,
		const World::Config &,
		const WorldSave &,
		const Server::Config &
	);

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

private:
	HeadlessEnvironment &env;
	BlockTypeRegistry block_types;
	World world;
	Server server;

};

}

#endif
