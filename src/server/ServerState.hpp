#ifndef BLANK_SERVER_SERVERSTATE_HPP_
#define BLANK_SERVER_SERVERSTATE_HPP_

#include "Server.hpp"
#include "../ai/Spawner.hpp"
#include "../app/IntervalTimer.hpp"
#include "../app/State.hpp"
#include "../model/Skeletons.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/ChunkIndex.hpp"
#include "../world/ChunkLoader.hpp"
#include "../world/Generator.hpp"
#include "../world/World.hpp"


namespace blank {

class Config;
class HeadlessEnvironment;
class WorldSave;

namespace server {

class ServerState
: public State {

public:
	ServerState(
		HeadlessEnvironment &,
		const Generator::Config &,
		const World::Config &,
		const WorldSave &,
		const Config &
	);
	~ServerState();

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

private:
	HeadlessEnvironment &env;
	BlockTypeRegistry block_types;
	World world;
	ChunkIndex &spawn_index;
	Generator generator;
	ChunkLoader chunk_loader;
	Skeletons skeletons;
	Spawner spawner;
	Server server;
	IntervalTimer loop_timer;

};

}
}

#endif
