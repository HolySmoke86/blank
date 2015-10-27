#ifndef BLANK_NET_CONNECTIONHANDLER_HPP_
#define BLANK_NET_CONNECTIONHANDLER_HPP_

#include "CongestionControl.hpp"
#include "Packet.hpp"

#include <SDL_net.h>


namespace blank {

class ConnectionHandler {

public:
	ConnectionHandler();

	const CongestionControl &NetStat() const noexcept { return cc; }

	void PacketSent(std::uint16_t) noexcept;
	void PacketLost(std::uint16_t);
	void PacketReceived(std::uint16_t);

	void PacketIn(const UDPpacket &) noexcept;
	void PacketOut(const UDPpacket &) noexcept;

	void Handle(const UDPpacket &);

	virtual void OnTimeout() { }

private:
	// called as soon as the remote end ack'd given packet
	virtual void OnPacketReceived(std::uint16_t) { }
	// called if the remote end probably didn't get given packet
	virtual void OnPacketLost(std::uint16_t) { }

	virtual void On(const Packet::Ping &) { }
	virtual void On(const Packet::Login &) { }
	virtual void On(const Packet::Join &) { }
	virtual void On(const Packet::Part &) { }
	virtual void On(const Packet::PlayerUpdate &) { }
	virtual void On(const Packet::SpawnEntity &) { }
	virtual void On(const Packet::DespawnEntity &) { }
	virtual void On(const Packet::EntityUpdate &) { }
	virtual void On(const Packet::PlayerCorrection &) { }
	virtual void On(const Packet::ChunkBegin &) { }
	virtual void On(const Packet::ChunkData &) { }
	virtual void On(const Packet::BlockUpdate &) { }
	virtual void On(const Packet::Message &) { }

private:
	CongestionControl cc;

};

}

#endif
