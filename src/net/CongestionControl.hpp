#ifndef BLANK_NET_CONGESTIONCONTROL_HPP_
#define BLANK_NET_CONGESTIONCONTROL_HPP_

#include <cstdint>
#include <SDL_net.h>


namespace blank {

class CongestionControl {

public:
	CongestionControl();

	/// packet loss as factor
	float PacketLoss() const noexcept { return packet_loss; }
	/// smooth average round trip time in milliseconds
	float RoundTripTime() const noexcept { return rtt; }
	/// estimated kilobytes transferred per second
	float Upstream() const noexcept { return tx_kbps; }
	/// estimated kilobytes received per second
	float Downstream() const noexcept { return rx_kbps; }

	void PacketSent(std::uint16_t) noexcept;
	void PacketLost(std::uint16_t) noexcept;
	void PacketReceived(std::uint16_t) noexcept;

	void PacketIn(const UDPpacket &) noexcept;
	void PacketOut(const UDPpacket &) noexcept;

private:
	void UpdatePacketLoss() noexcept;
	void UpdateRTT(std::uint16_t) noexcept;
	bool SamplePacket(std::uint16_t) const noexcept;
	int HeadDiff(std::uint16_t) const noexcept;
	void UpdateStats() noexcept;

private:
	const unsigned int packet_overhead;
	const unsigned int sample_skip;

	unsigned int packets_lost;
	unsigned int packets_received;
	float packet_loss;

	Uint32 stamps[16];
	std::size_t stamp_cursor;
	std::uint16_t stamp_last;
	float rtt;

	Uint32 next_sample;
	std::size_t tx_bytes;
	std::size_t rx_bytes;
	float tx_kbps;
	float rx_kbps;

};

}

#endif