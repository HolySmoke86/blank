#include "PreloadState.hpp"

#include "Environment.hpp"
#include "../world/ChunkLoader.hpp"


namespace blank {

PreloadState::PreloadState(Environment &env, ChunkLoader &loader)
: env(env)
, loader(loader)
, progress(env.assets.large_ui_font)
, total(loader.ToLoad())
, per_update(64) {
	progress.Position(glm::vec3(0.0f), Gravity::CENTER);
	progress.Template("Preloading chunks: %d/%d (%d%%)");
}


void PreloadState::Handle(const SDL_Event &e) {
	if (e.type == SDL_QUIT) {
		env.state.PopAll();
	}
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
