#include "UnloadState.hpp"

#include "../app/Environment.hpp"
#include "../io/WorldSave.hpp"
#include "../world/ChunkLoader.hpp"


namespace blank {
namespace standalone {

UnloadState::UnloadState(
	Environment &env,
	ChunkStore &chunks,
	const WorldSave &save)
: env(env)
, chunks(chunks)
, save(save)
, progress(env.assets.large_ui_font)
, cur(chunks.begin())
, end(chunks.end())
, done(0)
, total(chunks.NumLoaded())
, per_update(64) {
	progress.Position(glm::vec3(0.0f), Gravity::CENTER);
	progress.Template("Unloading chunks: %d/%d (%d%%)");
}


void UnloadState::OnResume() {
	cur = chunks.begin();
	end = chunks.end();
	done = 0;
	total = chunks.NumLoaded();
}


void UnloadState::Handle(const SDL_Event &) {
	// ignore everything
}

void UnloadState::Update(int dt) {
	for (std::size_t i = 0; i < per_update && cur != end; ++i, ++cur, ++done) {
		if (cur->ShouldUpdateSave()) {
			save.Write(*cur);
		}
	}
	if (cur == end) {
		env.state.PopAll();
	} else {
		progress.Update(done, total);
	}
}

void UnloadState::Render(Viewport &viewport) {
	progress.Render(viewport);
}

}
}
