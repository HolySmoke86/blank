#ifndef BLANK_CLIENT_INTERACTIVESTATE_HPP_
#define BLANK_CLIENT_INTERACTIVESTATE_HPP_

#include "../app/State.hpp"
#include "../io/WorldSave.hpp"
#include "../ui/Interface.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/ChunkRenderer.hpp"
#include "../world/World.hpp"


namespace blank {

class Environment;

namespace client {

class MasterState;

class InteractiveState
: public State {

public:
	explicit InteractiveState(MasterState &);

	World &GetWorld() noexcept { return world; }
	Interface &GetInterface() noexcept { return interface; }

	void OnEnter() override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

private:
	MasterState &master;
	BlockTypeRegistry block_types;
	WorldSave save;
	World world;
	ChunkRenderer chunk_renderer;
	Interface interface;

};

}
}

#endif
