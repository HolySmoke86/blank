#ifndef BLANK_NET_PACKETHANDLER_HPP_
#define BLANK_NET_PACKETHANDLER_HPP_

#include "Packet.hpp"

#include <SDL_net.h>


namespace blank {

class PacketHandler {

public:
	void Handle(const UDPpacket &);

private:
	virtual void On(const Packet::Ping &) { }
	virtual void On(const Packet::Login &) { }
	virtual void On(const Packet::Join &) { }
	virtual void On(const Packet::Part &) { }

};

}

#endif
