#ifndef BLANK_CLIENT_CLIENT_HPP_
#define BLANK_CLIENT_CLIENT_HPP_

#include "../net/Connection.hpp"

#include <string>
#include <SDL_net.h>


namespace blank {

class World;

namespace client {

class Client {

public:
	struct Config {
		std::string host = "localhost";
		Uint16 port = 12354;
	};

public:
	explicit Client(const Config &);
	~Client();

	void Handle();

	void Update(int dt);

	Connection &GetConnection() noexcept { return conn; }
	const Connection &GetConnection() const noexcept { return conn; }

	std::uint16_t SendPing();
	std::uint16_t SendLogin(const std::string &);
	std::uint16_t SendPart();
	std::uint16_t SendPlayerUpdate(const Entity &);

private:
	void HandlePacket(const UDPpacket &);

private:
	Connection conn;
	UDPsocket client_sock;
	UDPpacket client_pack;

};

}
}

#endif
