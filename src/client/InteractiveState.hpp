#ifndef BLANK_CLIENT_INTERACTIVESTATE_HPP_
#define BLANK_CLIENT_INTERACTIVESTATE_HPP_

#include "../app/State.hpp"
#include "../ui/ClientController.hpp"

#include "ChunkReceiver.hpp"
#include "ChunkRequester.hpp"
#include "../app/IntervalTimer.hpp"
#include "../graphics/SkyBox.hpp"
#include "../io/WorldSave.hpp"
#include "../model/Skeletons.hpp"
#include "../ui/DirectInput.hpp"
#include "../ui/HUD.hpp"
#include "../ui/InteractiveManipulator.hpp"
#include "../ui/Interface.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/ChunkRenderer.hpp"
#include "../world/EntityState.hpp"
#include "../world/Player.hpp"
#include "../world/World.hpp"

#include <list>


namespace blank {

class Environment;

namespace client {

class MasterState;

class InteractiveState
: public State
, public ClientController {

public:
	explicit InteractiveState(MasterState &, std::uint32_t player_id);

	World &GetWorld() noexcept { return world; }
	Player &GetPlayer() noexcept { return player; }
	ChunkReceiver &GetChunkReceiver() noexcept { return chunk_receiver; }
	Skeletons &GetSkeletons() noexcept { return skeletons; }

	void OnEnter() override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

	void PushPlayerUpdate(const Entity &, int dt);
	void MergePlayerCorrection(std::uint16_t, const EntityState &);

	void SetAudio(bool) override;
	void SetVideo(bool) override;
	void SetHUD(bool) override;
	void SetDebug(bool) override;
	void Exit() override;

private:
	MasterState &master;
	BlockTypeRegistry block_types;
	WorldSave save;
	World world;
	Player &player;
	HUD hud;
	InteractiveManipulator manip;
	DirectInput input;
	Interface interface;
	ChunkRequester chunk_requester;
	ChunkReceiver chunk_receiver;
	ChunkRenderer chunk_renderer;
	Skeletons skeletons;
	IntervalTimer loop_timer;

	SkyBox sky;

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
