#include "UnloadState.hpp"

#include "Environment.hpp"
#include "../world/ChunkLoader.hpp"
#include "../world/WorldSave.hpp"


namespace blank {

UnloadState::UnloadState(Environment &env, ChunkLoader &loader)
: env(env)
, loader(loader)
, font(env.assets.LoadFont("DejaVuSans", 24))
, progress(font)
, cur(loader.Loaded().begin())
, end(loader.Loaded().end())
, done(0)
, total(loader.Loaded().size())
, per_update(64) {
	progress.Position(glm::vec3(0.0f), Gravity::CENTER);
	progress.Template("Unloading chunks: %d/%d (%d%%)");
}


void UnloadState::OnResume() {
	cur = loader.Loaded().begin();
	end = loader.Loaded().end();
	done = 0;
	total = loader.Loaded().size();
}


void UnloadState::Handle(const SDL_Event &) {
	// ignore everything
}

void UnloadState::Update(int dt) {
	for (std::size_t i = 0; i < per_update && cur != end; ++i, ++cur, ++done) {
		if (cur->ShouldUpdateSave()) {
			loader.SaveFile().Write(*cur);
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
