#include "ServerState.hpp"

#include "../app/Environment.hpp"
#include "../app/TextureIndex.hpp"
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
, shapes()
, block_types()
, models()
, world(block_types, wc)
, generator(gc)
, chunk_loader(world.Chunks(), generator, ws)
, spawner(world, models, env.rng)
, server(config.net, world, wc, ws)
, loop_timer(16) {
	TextureIndex tex_index;
	env.loader.LoadShapes("default", shapes);
	env.loader.LoadBlockTypes("default", block_types, tex_index, shapes);
	env.loader.LoadModels("default", models, tex_index, shapes);
	if (models.size() < 2) {
		throw std::runtime_error("need at least two models to run");
	}
	generator.LoadTypes(block_types);
	spawner.LimitModels(1, models.size());
	server.SetPlayerModel(models[0]);

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
