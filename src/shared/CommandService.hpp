#ifndef BLANK_SHARED_COMMANDSERVICE_HPP_
#define BLANK_SHARED_COMMANDSERVICE_HPP_

#include "../net/tcp.hpp"


namespace blank {

class CLI;

class CommandService
: public tcp::IOHandler {

public:
	CommandService(CLI &, unsigned short port);
	~CommandService();

public:
	/// wait on incoming data for at most timeout ms
	void Wait(int timeout) noexcept;
	/// true if at least one connected socket can read
	bool Ready() noexcept;
	/// handle all inbound traffic
	void Handle();
	/// send all outbound traffic
	void Send();

	void OnRecv(tcp::Socket &) override;

private:
	CLI &cli;
	tcp::Pool pool;

};

}

#endif
