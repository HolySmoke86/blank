#ifndef BLANK_CLIENT_INTERACTIVESTATE_HPP_
#define BLANK_CLIENT_INTERACTIVESTATE_HPP_

#include "ChunkRequester.hpp"
#include "../app/IntervalTimer.hpp"
#include "../app/State.hpp"
#include "../io/WorldSave.hpp"
#include "../model/Skeletons.hpp"
#include "../net/ChunkReceiver.hpp"
#include "../ui/Interface.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/ChunkRenderer.hpp"
#include "../world/EntityState.hpp"
#include "../world/World.hpp"

#include <list>


namespace blank {

class Environment;

namespace client {

class MasterState;

class InteractiveState
: public State {

public:
	explicit InteractiveState(MasterState &, std::uint32_t player_id);

	World &GetWorld() noexcept { return world; }
	Interface &GetInterface() noexcept { return interface; }
	ChunkReceiver &GetChunkReceiver() noexcept { return chunk_receiver; }
	Skeletons &GetSkeletons() noexcept { return skeletons; }

	void OnEnter() override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

	void PushPlayerUpdate(const Entity &, int dt);
	void MergePlayerCorrection(std::uint16_t, const EntityState &);

private:
	MasterState &master;
	BlockTypeRegistry block_types;
	WorldSave save;
	World world;
	Interface interface;
	ChunkRequester chunk_requester;
	ChunkReceiver chunk_receiver;
	ChunkRenderer chunk_renderer;
	Skeletons skeletons;
	IntervalTimer loop_timer;

	struct PlayerHistory {
		EntityState state;
		int delta_t;
		std::uint16_t packet;
		PlayerHistory(EntityState s, int dt, std::uint16_t p)
		: state(s), delta_t(dt), packet(p) { }
	};
	std::list<PlayerHistory> player_hist;

};

}
}

#endif
