#ifndef BLANK_NET_CONNECTIONHANDLER_HPP_
#define BLANK_NET_CONNECTIONHANDLER_HPP_

#include "Packet.hpp"

#include <SDL_net.h>


namespace blank {

class ConnectionHandler {

public:
	void Handle(const UDPpacket &);

	virtual void OnPacketLost(std::uint16_t) { }

	virtual void OnTimeout() { }

private:
	virtual void On(const Packet::Ping &) { }
	virtual void On(const Packet::Login &) { }
	virtual void On(const Packet::Join &) { }
	virtual void On(const Packet::Part &) { }
	virtual void On(const Packet::PlayerUpdate &) { }

};

}

#endif
