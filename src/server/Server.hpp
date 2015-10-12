#ifndef BLANK_SERVER_SERVER_HPP
#define BLANK_SERVER_SERVER_HPP

#include "../app/Config.hpp"
#include "../world/World.hpp"
#include "../world/WorldManipulator.hpp"

#include <list>
#include <SDL_net.h>


namespace blank {

class ChunkIndex;
class Model;
class Player;
class WorldSave;

namespace server {

class ClientConnection;

class Server
: public WorldManipulator {

public:
	Server(const Config::Network &, World &, const World::Config &, const WorldSave &);
	~Server();

	void Handle();

	void Update(int dt);

	UDPsocket &GetSocket() noexcept { return serv_sock; }
	UDPpacket &GetPacket() noexcept { return serv_pack; }

	World &GetWorld() noexcept { return world; }
	const WorldSave &GetWorldSave() noexcept { return save; }

	void SetPlayerModel(const Model &) noexcept;
	bool HasPlayerModel() const noexcept;
	const Model &GetPlayerModel() const noexcept;

	Player *JoinPlayer(const std::string &name);

	void SetBlock(Chunk &, int, const Block &) override;

private:
	void HandlePacket(const UDPpacket &);

	ClientConnection &GetClient(const IPaddress &);

private:
	UDPsocket serv_sock;
	UDPpacket serv_pack;
	std::list<ClientConnection> clients;

	World &world;
	ChunkIndex &spawn_index;
	const WorldSave &save;
	const Model *player_model;

};

}
}

#endif
