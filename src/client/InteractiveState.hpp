#ifndef BLANK_CLIENT_INTERACTIVESTATE_HPP_
#define BLANK_CLIENT_INTERACTIVESTATE_HPP_

#include "../app/State.hpp"
#include "../ui/ClientController.hpp"

#include "ChunkReceiver.hpp"
#include "NetworkedInput.hpp"
#include "../app/IntervalTimer.hpp"
#include "../graphics/SkyBox.hpp"
#include "../io/WorldSave.hpp"
#include "../model/Skeletons.hpp"
#include "../net/Packet.hpp"
#include "../ui/HUD.hpp"
#include "../ui/InteractiveManipulator.hpp"
#include "../ui/Interface.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/ChunkRenderer.hpp"
#include "../world/EntityState.hpp"
#include "../world/Player.hpp"
#include "../world/World.hpp"


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

	void MergePlayerCorrection(std::uint16_t, const EntityState &);
	void Handle(const Packet::BlockUpdate &);

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
	NetworkedInput input;
	Interface interface;
	ChunkReceiver chunk_receiver;
	ChunkRenderer chunk_renderer;
	Skeletons skeletons;
	IntervalTimer loop_timer;

	SkyBox sky;

};

}
}

#endif
