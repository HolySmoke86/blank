#include "PreloadState.hpp"

#include "Environment.hpp"
#include "../world/ChunkLoader.hpp"

#include <iostream>


namespace blank {

PreloadState::PreloadState(Environment &env, ChunkLoader &loader)
: env(env)
, loader(loader)
, per_update(256) {

}


void PreloadState::Handle(const SDL_Event &) {
}

void PreloadState::Update(int dt) {
	loader.LoadN(per_update);
	if (loader.ToLoad() == 0) {
		std::cout << "preload: populating VBOs" << std::endl;
		for (auto &chunk : loader.Loaded()) {
			chunk.CheckUpdate();
		}
		std::cout << "preload: complete" << std::endl;
		env.state.Pop();
	}
}

void PreloadState::Render(Viewport &) {
	// TODO: make a nice progress bar or some other fancy shit
	std::cout << "preload: " << loader.ToLoad() << " chunks remaining" << std::endl;
}

}
