#ifndef BLANK_NET_CONGESTIONCONTROL_HPP_
#define BLANK_NET_CONGESTIONCONTROL_HPP_

#include <cstdint>
#include <SDL_net.h>


namespace blank {

class CongestionControl {

public:
	enum Mode {
		GOOD,
		BAD,
		UGLY,
	};

public:
	CongestionControl();

	/// get recommended mode of operation
	Mode GetMode() const noexcept { return mode; }
	/// according to current mode, drop this many unimportant packets
	unsigned int SuggestedPacketSkip() const noexcept { return (1 << mode) - 1; }
	/// according to current mode, pause between large uncritical packets for this many ticks
	unsigned int SuggestedPacketHold() const noexcept { return (1 << (mode + 1)) - 1; }

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
	std::size_t SampleIndex(std::uint16_t) const noexcept;

	void UpdateStats() noexcept;

	void UpdateMode() noexcept;
	void CheckUpgrade(Mode) noexcept;
	void ChangeMode(Mode) noexcept;
	void KeepMode() noexcept;

	Mode Conditions() const noexcept;

private:
	const unsigned int packet_overhead;
	const unsigned int sample_skip;

	unsigned int packets_lost;
	unsigned int packets_received;
	float packet_loss;

	Uint32 stamps[16];
	std::uint16_t stamp_last;
	float rtt;

	Uint32 next_sample;
	std::size_t tx_bytes;
	std::size_t rx_bytes;
	float tx_kbps;
	float rx_kbps;

	Mode mode;
	float bad_rtt;
	float bad_loss;
	float ugly_rtt;
	float ugly_loss;
	Uint32 mode_entered;
	Uint32 mode_reset;
	Uint32 mode_keep_time;
	Uint32 mode_step;

};

}

#endif
