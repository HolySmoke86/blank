#ifndef BLANK_NET_CONNECTIONHANDLER_HPP_
#define BLANK_NET_CONNECTIONHANDLER_HPP_

#include "Packet.hpp"

#include <SDL_net.h>


namespace blank {

class ConnectionHandler {

public:
	ConnectionHandler();

	/// packet loss as factor
	float PacketLoss() const noexcept { return packet_loss; }
	/// smooth average round trip time in milliseconds
	float RoundTripTime() const noexcept { return rtt; }

	void PacketSent(std::uint16_t);
	void PacketLost(std::uint16_t);
	void PacketReceived(std::uint16_t);

	void Handle(const UDPpacket &);

	virtual void OnTimeout() { }

private:
	void UpdatePacketLoss() noexcept;
	void UpdateRTT(std::uint16_t) noexcept;
	bool SamplePacket(std::uint16_t) const noexcept;
	int HeadDiff(std::uint16_t) const noexcept;

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
	unsigned int packets_lost;
	unsigned int packets_received;
	float packet_loss;

	Uint32 stamps[16];
	std::size_t stamp_cursor;
	std::uint16_t stamp_last;
	float rtt;

};

}

#endif
