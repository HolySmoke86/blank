#ifndef BLANK_NET_CONNECTION_HPP_
#define BLANK_NET_CONNECTION_HPP_

#include "Packet.hpp"
#include "../app/IntervalTimer.hpp"

#include <cstdint>
#include <SDL_net.h>


namespace blank {

class Connection {

public:
	explicit Connection(const IPaddress &);

	const IPaddress &Address() const noexcept { return addr; }

	bool Matches(const IPaddress &) const noexcept;

	bool ShouldPing() const noexcept;
	bool TimedOut() const noexcept;

	void Close() noexcept { closed = true; }
	bool Closed() const noexcept { return closed || TimedOut(); }

	void Update(int dt);

	void SendPing(UDPpacket &, UDPsocket);

	void Send(UDPpacket &, UDPsocket);
	void Received(const UDPpacket &);

private:
	void FlagSend() noexcept;
	void FlagRecv() noexcept;

private:
	IPaddress addr;
	IntervalTimer send_timer;
	IntervalTimer recv_timer;

	Packet::TControl ctrl;

	bool closed;

};

}

#endif
