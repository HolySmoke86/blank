#ifndef BLANK_SERVER_SERVER_HPP
#define BLANK_SERVER_SERVER_HPP

#include "../app/Config.hpp"
#include "../shared/CLI.hpp"
#include "../world/World.hpp"
#include "../world/WorldManipulator.hpp"

#include <cstdint>
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

	/// for use by client connections when they receive a line from the player
	void DispatchMessage(Player &, const std::string &);

	/// send message to all connected clients
	void DistributeMessage(std::uint8_t type, std::uint32_t ref, const std::string &msg);

private:
	void HandlePacket(const UDPpacket &);

	ClientConnection &GetClient(const IPaddress &);

	void SendAll();

private:
	UDPsocket serv_sock;
	UDPpacket serv_pack;
	std::list<ClientConnection> clients;

	World &world;
	ChunkIndex &spawn_index;
	const WorldSave &save;
	const Model *player_model;

	CLI cli;

};

}
}

#endif
