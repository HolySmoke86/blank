#ifndef BLANK_NET_SERVER_HPP
#define BLANK_NET_SERVER_HPP

#include <list>
#include <SDL_net.h>


namespace blank {

class ClientConnection;
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

	UDPsocket &GetSocket() noexcept { return serv_sock; }
	UDPpacket &GetPacket() noexcept { return serv_pack; }

	World &GetWorld() noexcept { return world; }

private:
	void HandlePacket(const UDPpacket &);

	ClientConnection &GetClient(const IPaddress &);

private:
	UDPsocket serv_sock;
	UDPpacket serv_pack;
	std::list<ClientConnection> clients;

	World &world;

};

}

#endif
