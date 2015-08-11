#ifndef BLANK_APP_WORLDSTATE_HPP_
#define BLANK_APP_WORLDSTATE_HPP_

#include "PreloadState.hpp"
#include "State.hpp"
#include "UnloadState.hpp"
#include "../ai/Spawner.hpp"
#include "../ui/Interface.hpp"
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

private:
	Environment &env;
	World world;
	Spawner spawner;
	Interface interface;

	PreloadState preload;
	UnloadState unload;

};

}

#endif
