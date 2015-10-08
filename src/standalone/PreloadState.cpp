#include "PreloadState.hpp"

#include "../app/Environment.hpp"
#include "../world/ChunkLoader.hpp"
#include "../world/ChunkRenderer.hpp"


namespace blank {
namespace standalone {

PreloadState::PreloadState(Environment &env, ChunkLoader &loader, ChunkRenderer &render)
: ProgressState(env, "Preloading chunks: %d/%d (%d%%)")
, env(env)
, loader(loader)
, render(render)
, total(loader.ToLoad())
, per_update(64) {

}

void PreloadState::Update(int dt) {
	loader.LoadN(per_update);
	if (loader.ToLoad() <= 0) {
		env.state.Pop();
		render.Update(render.MissingChunks());
	} else {
		SetProgress(total - loader.ToLoad(), total);
	}
}

}
}
