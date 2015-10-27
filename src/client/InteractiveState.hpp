#ifndef BLANK_CLIENT_INTERACTIVESTATE_HPP_
#define BLANK_CLIENT_INTERACTIVESTATE_HPP_

#include "../app/State.hpp"
#include "../ui/ClientController.hpp"

#include "ChunkReceiver.hpp"
#include "NetworkedInput.hpp"
#include "../app/IntervalTimer.hpp"
#include "../audio/SoundBank.hpp"
#include "../graphics/SkyBox.hpp"
#include "../io/WorldSave.hpp"
#include "../net/Packet.hpp"
#include "../shared/ChatState.hpp"
#include "../shared/WorldResources.hpp"
#include "../ui/HUD.hpp"
#include "../ui/InteractiveManipulator.hpp"
#include "../ui/Interface.hpp"
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
, public ClientController
, public ChatState::Responder {

public:
	explicit InteractiveState(MasterState &, std::uint32_t player_id);

	World &GetWorld() noexcept { return world; }
	Player &GetPlayer() noexcept { return player; }
	PlayerController &GetPlayerController() noexcept { return input; }
	ChunkReceiver &GetChunkReceiver() noexcept { return chunk_receiver; }

	void OnResume() override;
	void OnPause() override;

	void OnFocus() override;
	void OnBlur() override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

	void Handle(const Packet::SpawnEntity &);
	void Handle(const Packet::DespawnEntity &);
	void Handle(const Packet::EntityUpdate &);
	void Handle(const Packet::PlayerCorrection &);
	void Handle(const Packet::BlockUpdate &);
	void Handle(const Packet::Message &);

	void SetAudio(bool) override;
	void SetVideo(bool) override;
	void SetHUD(bool) override;
	void SetDebug(bool) override;
	void Exit() override;

	void OnLineSubmit(const std::string &) override;

private:
	/// flag entity as updated by given packet
	/// returns false if the update should be ignored
	bool UpdateEntity(std::uint32_t id, std::uint16_t seq);
	/// drop update information or given entity
	void ClearEntity(std::uint32_t id);

private:
	MasterState &master;
	WorldResources res;
	SoundBank sounds;
	WorldSave save;
	World world;
	Player &player;
	HUD hud;
	InteractiveManipulator manip;
	NetworkedInput input;
	Interface interface;
	ChunkReceiver chunk_receiver;
	ChunkRenderer chunk_renderer;
	CoarseTimer loop_timer;
	CoarseTimer stat_timer;

	SkyBox sky;

	struct UpdateStatus {
		std::uint16_t last_packet;
		int last_update;
	};
	std::map<std::uint32_t, UpdateStatus> update_status;

	ChatState chat;

	int time_skipped;
	unsigned int packets_skipped;

};

}
}

#endif
