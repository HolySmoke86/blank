#ifndef BLANK_CLIENT_INTERACTIVESTATE_HPP_
#define BLANK_CLIENT_INTERACTIVESTATE_HPP_

#include "../app/IntervalTimer.hpp"
#include "../app/State.hpp"
#include "../io/WorldSave.hpp"
#include "../model/Skeletons.hpp"
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
	Skeletons &GetSkeletons() noexcept { return skeletons; }

	void OnEnter() override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

	void PushPlayerUpdate(const Entity &);
	void MergePlayerCorrection(std::uint16_t, const EntityState &);

private:
	MasterState &master;
	BlockTypeRegistry block_types;
	WorldSave save;
	World world;
	Interface interface;
	ChunkRenderer chunk_renderer;
	Skeletons skeletons;
	IntervalTimer update_timer;

	struct PlayerHistory {
		EntityState state;
		int timestamp;
		std::uint16_t packet;
		PlayerHistory(EntityState s, int t, std::uint16_t p)
		: state(s), timestamp(t), packet(p) { }
	};
	std::list<PlayerHistory> player_hist;

};

}
}

#endif
