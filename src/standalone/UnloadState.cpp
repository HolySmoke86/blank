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
: ProgressState(env, "Unloading chunks: %d/%d (%d%%)")
, env(env)
, chunks(chunks)
, save(save)
, cur(chunks.begin())
, end(chunks.end())
, done(0)
, total(chunks.NumLoaded())
, per_update(64) {

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
		env.state.Pop();
	} else {
		SetProgress(done, total);
	}
}

}
}
