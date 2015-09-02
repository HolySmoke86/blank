#ifndef BLANK_NET_CLIENT_HPP_
#define BLANK_NET_CLIENT_HPP_

#include "Connection.hpp"

#include <string>
#include <SDL_net.h>


namespace blank {

class World;

class Client {

public:
	struct Config {
		std::string host = "localhost";
		Uint16 port = 12354;
	};

public:
	Client(const Config &, World &);
	~Client();

	void Handle();

	void Update(int dt);

	bool TimedOut() { return conn.TimedOut(); }

	void SendPing();
	void SendLogin(const std::string &);

private:
	void HandlePacket(const UDPpacket &);

private:
	World &world;
	Connection conn;
	UDPsocket client_sock;
	UDPpacket client_pack;

};

}

#endif
