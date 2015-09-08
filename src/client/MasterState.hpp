#ifndef BLANK_CLIENT_CLIENTSTATE_HPP_
#define BLANK_CLIENT_CLIENTSTATE_HPP_

#include "InitialState.hpp"
#include "InteractiveState.hpp"
#include "../app/State.hpp"
#include "../net/Client.hpp"
#include "../net/ConnectionHandler.hpp"

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
		const World::Config &,
		const Interface::Config &,
		const Client::Config &
	);

	Client &GetClient() noexcept { return client; }
	Environment &GetEnv() noexcept { return env; }

	World::Config &GetWorldConf() noexcept { return world_conf; }
	const World::Config &GetWorldConf() const noexcept { return world_conf; }
	const Interface::Config &GetInterfaceConf() const noexcept { return intf_conf; }
	const Client::Config &GetClientConf() const noexcept { return client_conf; }

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

private:
	Environment &env;
	World::Config world_conf;
	const Interface::Config &intf_conf;
	const Client::Config &client_conf;
	std::unique_ptr<InteractiveState> state;
	Client client;

	InitialState init_state;

	int login_packet;

};

}
}

#endif
