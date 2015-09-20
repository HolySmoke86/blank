#ifndef BLANK_NET_CONNECTIONHANDLER_HPP_
#define BLANK_NET_CONNECTIONHANDLER_HPP_

#include "Packet.hpp"

#include <SDL_net.h>


namespace blank {

class ConnectionHandler {

public:
	ConnectionHandler();

	float PacketLoss() const noexcept { return packet_loss; }

	void PacketLost(std::uint16_t);
	void PacketReceived(std::uint16_t);

	void Handle(const UDPpacket &);

	virtual void OnTimeout() { }

private:
	void UpdatePacketLoss() noexcept;

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

private:
	unsigned int packets_lost;
	unsigned int packets_received;
	float packet_loss;

};

}

#endif
