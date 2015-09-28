#ifndef BLANK_STANDALONE_MASTERSTATE_HPP_
#define BLANK_STANDALONE_MASTERSTATE_HPP_

#include "../app/State.hpp"

#include "PreloadState.hpp"
#include "UnloadState.hpp"
#include "../ai/Spawner.hpp"
#include "../graphics/SkyBox.hpp"
#include "../model/Skeletons.hpp"
#include "../ui/Interface.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/ChunkLoader.hpp"
#include "../world/ChunkRenderer.hpp"
#include "../world/Generator.hpp"
#include "../world/World.hpp"


namespace blank {

class Environment;

namespace standalone {

class MasterState
: public State {

public:
	MasterState(
		Environment &,
		const Generator::Config &,
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
	Interface interface;
	Generator generator;
	ChunkLoader chunk_loader;
	ChunkRenderer chunk_renderer;
	Skeletons skeletons;
	Spawner spawner;

	SkyBox sky;

	PreloadState preload;
	UnloadState unload;

};

}
}

#endif
