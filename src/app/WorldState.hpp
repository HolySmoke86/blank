#ifndef BLANK_APP_WORLDSTATE_HPP_
#define BLANK_APP_WORLDSTATE_HPP_

#include "PreloadState.hpp"
#include "State.hpp"
#include "UnloadState.hpp"
#include "../ai/Spawner.hpp"
#include "../model/Skeletons.hpp"
#include "../ui/Interface.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/ChunkRenderer.hpp"
#include "../world/World.hpp"


namespace blank {

class Environment;

class WorldState
: public State {

public:
	WorldState(
		Environment &,
		const Interface::Config &,
		const World::Config &,
		const WorldSave &
	);

	void OnEnter() override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

	World &GetWorld() noexcept { return world; }
	Interface &GetInterface() noexcept { return interface; }

private:
	Environment &env;
	BlockTypeRegistry block_types;
	World world;
	ChunkRenderer chunk_renderer;
	Skeletons skeletons;
	Spawner spawner;
	Interface interface;

	PreloadState preload;
	UnloadState unload;

};

}

#endif
