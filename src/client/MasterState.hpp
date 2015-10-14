#ifndef BLANK_CLIENT_CLIENTSTATE_HPP_
#define BLANK_CLIENT_CLIENTSTATE_HPP_

#include "../app/State.hpp"
#include "../net/ConnectionHandler.hpp"

#include "Client.hpp"
#include "InitialState.hpp"
#include "InteractiveState.hpp"
#include "../app/Config.hpp"

#include <map>
#include <memory>


namespace blank {

class Environment;

namespace client {

class InteractiveState;

class MasterState
: public State
, public ConnectionHandler {

public:
	MasterState(
		Environment &,
		Config &,
		const World::Config &
	);

	Client &GetClient() noexcept { return client; }
	Environment &GetEnv() noexcept { return env; }

	Config &GetConfig() noexcept { return config; }
	const Config &GetConfig() const noexcept { return config; }

	World::Config &GetWorldConf() noexcept { return world_conf; }
	const World::Config &GetWorldConf() const noexcept { return world_conf; }

	void Quit();

	void OnEnter() override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

	void OnPacketLost(std::uint16_t) override;
	void OnTimeout() override;

	void On(const Packet::Join &) override;
	void On(const Packet::Part &) override;
	void On(const Packet::SpawnEntity &) override;
	void On(const Packet::DespawnEntity &) override;
	void On(const Packet::EntityUpdate &) override;
	void On(const Packet::PlayerCorrection &) override;
	void On(const Packet::ChunkBegin &) override;
	void On(const Packet::ChunkData &) override;
	void On(const Packet::BlockUpdate &) override;

private:
	Environment &env;
	Config &config;
	World::Config world_conf;
	std::unique_ptr<InteractiveState> state;
	Client client;

	InitialState init_state;

	int login_packet;

};

}
}

#endif
