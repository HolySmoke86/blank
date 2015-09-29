#ifndef BLANK_SERVER_SERVER_HPP
#define BLANK_SERVER_SERVER_HPP

#include "../app/Config.hpp"

#include <list>
#include <SDL_net.h>


namespace blank {

class CompositeModel;
class World;

namespace server {

class ClientConnection;

class Server {

public:
	Server(const Config::Network &, World &);
	~Server();

	void Handle();

	void Update(int dt);

	UDPsocket &GetSocket() noexcept { return serv_sock; }
	UDPpacket &GetPacket() noexcept { return serv_pack; }

	World &GetWorld() noexcept { return world; }

	void SetPlayerModel(const CompositeModel &) noexcept;
	bool HasPlayerModel() const noexcept;
	const CompositeModel &GetPlayerModel() const noexcept;

private:
	void HandlePacket(const UDPpacket &);

	ClientConnection &GetClient(const IPaddress &);

private:
	UDPsocket serv_sock;
	UDPpacket serv_pack;
	std::list<ClientConnection> clients;

	World &world;
	const CompositeModel *player_model;

};

}
}

#endif
