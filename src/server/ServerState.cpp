#include "ServerState.hpp"

#include "../app/Environment.hpp"
#include "../io/WorldSave.hpp"
#include "../net/io.hpp"

#include <iostream>


namespace blank {
namespace server {

ServerState::ServerState(
	HeadlessEnvironment &env,
	const Generator::Config &gc,
	const World::Config &wc,
	const WorldSave &ws,
	const Config &config
)
: env(env)
, res()
, world(res.block_types, wc)
, generator(gc)
, chunk_loader(world.Chunks(), generator, ws)
, spawner(world, res.models)
, server(config.net, world, wc, ws)
, loop_timer(16) {
	res.Load(env.loader, "default");
	if (res.models.size() < 2) {
		throw std::runtime_error("need at least two models to run");
	}
	generator.LoadTypes(res.block_types);
	spawner.LimitModels(1, res.models.size());
	server.SetPlayerModel(res.models[0]);

	loop_timer.Start();

	std::cout << "listening on UDP port " << config.net.port << std::endl;
}

ServerState::~ServerState() {

}


void ServerState::Handle(const SDL_Event &event) {
	if (event.type == SDL_QUIT) {
		std::cout << "saving remaining chunks" << std::endl;
		for (Chunk &chunk : world.Chunks()) {
			if (chunk.ShouldUpdateSave()) {
				chunk_loader.SaveFile().Write(chunk);
			}
		}
		env.state.PopAll();
	}
}


void ServerState::Update(int dt) {
	loop_timer.Update(dt);
	if (!loop_timer.HitOnce() && loop_timer.IntervalRemain() > 1) {
		server.Wait(loop_timer.IntervalRemain() - 1);
		return;
	}
	if (dt == 0 && !server.Ready()) {
		// effectively wait in a spin loop
		return;
	}

	server.Handle();
	int world_dt = 0;
	while (loop_timer.HitOnce()) {
		spawner.Update(loop_timer.Interval());
		world.Update(loop_timer.Interval());
		world_dt += loop_timer.Interval();
		loop_timer.PopIteration();
	}
	chunk_loader.Update(dt);
	if (world_dt > 0) {
		server.Update(world_dt);
	}
	if (world_dt > 32) {
		std::cout << "world dt at " << world_dt << "ms!" << std::endl;
	}
}


void ServerState::Render(Viewport &) {

}

}
}
