#ifndef BLANK_STANDALONE_MASTERSTATE_HPP_
#define BLANK_STANDALONE_MASTERSTATE_HPP_

#include "../app/State.hpp"
#include "../ui/ClientController.hpp"

#include "PreloadState.hpp"
#include "UnloadState.hpp"
#include "../ai/Spawner.hpp"
#include "../graphics/SkyBox.hpp"
#include "../model/Skeletons.hpp"
#include "../ui/DirectInput.hpp"
#include "../ui/HUD.hpp"
#include "../ui/InteractiveManipulator.hpp"
#include "../ui/Interface.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/ChunkIndex.hpp"
#include "../world/ChunkLoader.hpp"
#include "../world/ChunkRenderer.hpp"
#include "../world/Generator.hpp"
#include "../world/Player.hpp"
#include "../world/World.hpp"


namespace blank {

class Config;
class Environment;

namespace standalone {

class MasterState
: public State
, public ClientController {

public:
	MasterState(
		Environment &,
		Config &,
		const Generator::Config &,
		const World::Config &,
		const WorldSave &
	);
	~MasterState();

	void OnResume() override;
	void OnPause() override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

	World &GetWorld() noexcept { return world; }
	Interface &GetInterface() noexcept { return interface; }

	void SetAudio(bool) override;
	void SetVideo(bool) override;
	void SetHUD(bool) override;
	void SetDebug(bool) override;
	void Exit() override;

private:
	Config &config;
	Environment &env;
	BlockTypeRegistry block_types;
	const WorldSave &save;
	World world;
	ChunkIndex &spawn_index;
	Player &player;
	bool spawn_player;
	HUD hud;
	InteractiveManipulator manip;
	DirectInput input;
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
