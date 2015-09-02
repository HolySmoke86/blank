#ifndef BLANK_NET_SERVER_HPP
#define BLANK_NET_SERVER_HPP

#include <list>
#include <SDL_net.h>


namespace blank {

class Connection;
class World;

class Server {

public:
	struct Config {
		Uint16 port = 12354;
	};

public:
	Server(const Config &, World &);
	~Server();

	void Handle();

	void Update(int dt);

private:
	void HandlePacket(const UDPpacket &);

	Connection &GetClient(const IPaddress &);

	void OnConnect(Connection &);
	void OnDisconnect(Connection &);

	void HandleLogin(Connection &client, const UDPpacket &);

private:
	UDPsocket serv_sock;
	UDPpacket serv_pack;
	std::list<Connection> clients;

	World &world;

};

}

#endif
