#include "PreloadState.hpp"

#include "Environment.hpp"
#include "../world/ChunkLoader.hpp"
#include "../world/ChunkRenderer.hpp"


namespace blank {

PreloadState::PreloadState(Environment &env, ChunkLoader &loader, ChunkRenderer &render)
: env(env)
, loader(loader)
, render(render)
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
	if (loader.ToLoad() <= 0) {
		env.state.Pop();
		render.Update(render.MissingChunks());
	} else {
		progress.Update(total - loader.ToLoad(), total);
	}
}

void PreloadState::Render(Viewport &viewport) {
	progress.Render(viewport);
}

}
