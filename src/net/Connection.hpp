#ifndef BLANK_NET_CONNECTION_HPP_
#define BLANK_NET_CONNECTION_HPP_

#include "Packet.hpp"
#include "../app/IntervalTimer.hpp"

#include <cstdint>
#include <SDL_net.h>


namespace blank {

class ConnectionHandler;

class Connection {

public:
	explicit Connection(const IPaddress &);

	void SetHandler(ConnectionHandler *h) noexcept { handler = h; }
	void RemoveHandler() noexcept { handler = nullptr; }
	bool HasHandler() const noexcept { return handler; }
	ConnectionHandler &Handler() noexcept { return *handler; }

	const IPaddress &Address() const noexcept { return addr; }

	bool Matches(const IPaddress &) const noexcept;

	bool ShouldPing() const noexcept;
	bool TimedOut() const noexcept;

	void Close() noexcept { closed = true; }
	bool Closed() const noexcept { return closed; }

	void Update(int dt);

	std::uint16_t SendPing(UDPpacket &, UDPsocket);

	std::uint16_t Send(UDPpacket &, UDPsocket);
	void Received(const UDPpacket &);

private:
	void FlagSend() noexcept;
	void FlagRecv() noexcept;

private:
	ConnectionHandler *handler;
	IPaddress addr;
	CoarseTimer send_timer;
	CoarseTimer recv_timer;

	Packet::TControl ctrl_out;
	Packet::TControl ctrl_in;

	bool closed;

};

}

#endif
