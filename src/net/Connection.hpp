#ifndef BLANK_NET_CONNECTION_HPP_
#define BLANK_NET_CONNECTION_HPP_

#include "../app/IntervalTimer.hpp"

#include <SDL_net.h>


namespace blank {

class Connection {

public:
	explicit Connection(const IPaddress &);

	const IPaddress &Address() const noexcept { return addr; }

	bool Matches(const IPaddress &) const noexcept;

	void FlagSend() noexcept;
	void FlagRecv() noexcept;

	bool ShouldPing() const noexcept;
	bool TimedOut() const noexcept;

	void Update(int dt);


	void SendPing(UDPpacket &, UDPsocket);

	void Send(UDPpacket &, UDPsocket);

private:
	IPaddress addr;
	IntervalTimer send_timer;
	IntervalTimer recv_timer;

};

}

#endif
