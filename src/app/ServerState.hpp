#ifndef BLANK_APP_SERVERSTATE_HPP_
#define BLANK_APP_SERVERSTATE_HPP_

#include "IntervalTimer.hpp"
#include "State.hpp"
#include "../ai/Spawner.hpp"
#include "../model/Skeletons.hpp"
#include "../net/Server.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/ChunkLoader.hpp"
#include "../world/Generator.hpp"
#include "../world/World.hpp"


namespace blank {

class HeadlessEnvironment;
class WorldSave;

class ServerState
: public State {

public:
	ServerState(
		HeadlessEnvironment &,
		const Generator::Config &,
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
	Generator generator;
	ChunkLoader chunk_loader;
	Skeletons skeletons;
	Spawner spawner;
	Server server;
	IntervalTimer loop_timer;
	IntervalTimer push_timer;

};

}

#endif
