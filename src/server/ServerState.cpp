#include "ServerState.hpp"

#include "../app/Environment.hpp"
#include "../app/TextureIndex.hpp"
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
, block_types()
, world(block_types, wc)
, spawn_index(world.Chunks().MakeIndex(wc.spawn, 3))
, generator(gc, block_types)
, chunk_loader(world.Chunks(), generator, ws)
, skeletons()
, spawner(world, skeletons, env.rng)
, server(config.net, world, ws)
, loop_timer(16) {
	TextureIndex tex_index;
	env.loader.LoadBlockTypes("default", block_types, tex_index);
	generator.Scan();
	skeletons.LoadHeadless();
	spawner.LimitSkeletons(1, skeletons.size());
	server.SetPlayerModel(skeletons[0]);

	loop_timer.Start();

	std::cout << "listening on UDP port " << config.net.port << std::endl;
}

ServerState::~ServerState() {
	world.Chunks().UnregisterIndex(spawn_index);
}


void ServerState::Handle(const SDL_Event &event) {
	if (event.type == SDL_QUIT) {
		env.state.PopAll();
	}
}


void ServerState::Update(int dt) {
	loop_timer.Update(dt);
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
}


void ServerState::Render(Viewport &viewport) {

}

}
}
