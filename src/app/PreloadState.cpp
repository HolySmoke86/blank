#include "PreloadState.hpp"

#include "Environment.hpp"
#include "../world/ChunkLoader.hpp"

#include <iostream>


namespace blank {

PreloadState::PreloadState(Environment &env, ChunkLoader &loader)
: env(env)
, loader(loader)
, font(env.assets.LoadFont("DejaVuSans", 24))
, progress(font)
, total(loader.ToLoad())
, per_update(64) {
	progress.Position(glm::vec3(0.0f), Gravity::CENTER);
	progress.Template("Preloading chunks: %d/%d (%d%%)");
}


void PreloadState::Handle(const SDL_Event &) {
}

void PreloadState::Update(int dt) {
	loader.LoadN(per_update);
	if (loader.ToLoad() == 0) {
		for (auto &chunk : loader.Loaded()) {
			chunk.CheckUpdate();
		}
		env.state.Pop();
	} else {
		progress.Update(total - loader.ToLoad(), total);
	}
}

void PreloadState::Render(Viewport &viewport) {
	progress.Render(viewport);
}

}
