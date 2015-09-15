#include "ServerState.hpp"

#include "Environment.hpp"
#include "TextureIndex.hpp"
#include "../net/io.hpp"

#include <iostream>


namespace blank {

ServerState::ServerState(
	HeadlessEnvironment &env,
	const Generator::Config &gc,
	const World::Config &wc,
	const WorldSave &ws,
	const Server::Config &sc
)
: env(env)
, block_types()
, world(block_types, wc)
, generator(gc)
, chunk_loader(world.Chunks(), generator, ws)
, skeletons()
, spawner(world, skeletons, gc.seed)
, server(sc, world)
, loop_timer(16)
, push_timer(16) {
	TextureIndex tex_index;
	env.loader.LoadBlockTypes("default", block_types, tex_index);
	skeletons.LoadHeadless();

	loop_timer.Start();
	push_timer.Start();

	std::cout << "listening on UDP port " << sc.port << std::endl;
}


void ServerState::Handle(const SDL_Event &event) {
	if (event.type == SDL_QUIT) {
		env.state.PopAll();
	}
}


void ServerState::Update(int dt) {
	loop_timer.Update(dt);
	push_timer.Update(dt);
	server.Handle();
	while (loop_timer.HitOnce()) {
		spawner.Update(loop_timer.Interval());
		world.Update(loop_timer.Interval());
		loop_timer.PopIteration();
	}
	chunk_loader.Update(dt);
	if (push_timer.Hit()) {
		server.Update(dt);
	}
}


void ServerState::Render(Viewport &viewport) {

}

}
