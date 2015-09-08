#ifndef BLANK_NET_CONNECTIONHANDLER_HPP_
#define BLANK_NET_CONNECTIONHANDLER_HPP_

#include "Packet.hpp"

#include <SDL_net.h>


namespace blank {

class ConnectionHandler {

public:
	void Handle(const UDPpacket &);

	// called as soon as the remote end ack'd given packet
	virtual void OnPacketReceived(std::uint16_t) { }
	// called if the remote end probably didn't get given packet
	virtual void OnPacketLost(std::uint16_t) { }

	virtual void OnTimeout() { }

private:
	virtual void On(const Packet::Ping &) { }
	virtual void On(const Packet::Login &) { }
	virtual void On(const Packet::Join &) { }
	virtual void On(const Packet::Part &) { }
	virtual void On(const Packet::PlayerUpdate &) { }
	virtual void On(const Packet::SpawnEntity &) { }
	virtual void On(const Packet::DespawnEntity &) { }
	virtual void On(const Packet::EntityUpdate &) { }

};

}

#endif
