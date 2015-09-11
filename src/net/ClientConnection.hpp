#ifndef BLANK_NET_CLIENTCONNECTION_HPP_
#define BLANK_NET_CLIENTCONNECTION_HPP_

#include "Connection.hpp"
#include "ConnectionHandler.hpp"
#include "../app/IntervalTimer.hpp"

#include <list>
#include <SDL_net.h>


namespace blank {

class Entity;
class Server;

class ClientConnection
: public ConnectionHandler {

public:
	explicit ClientConnection(Server &, const IPaddress &);
	~ClientConnection();

	bool Matches(const IPaddress &addr) const noexcept { return conn.Matches(addr); }

	void Update(int dt);

	Connection &GetConnection() noexcept { return conn; }
	bool Disconnected() const noexcept { return conn.Closed(); }

	void AttachPlayer(Entity &);
	void DetachPlayer();
	bool HasPlayer() const noexcept { return player; }
	Entity &Player() noexcept { return *player; }
	const Entity &Player() const noexcept { return *player; }

private:
	struct SpawnStatus {
		// the entity in question
		Entity *const entity = nullptr;
		// sequence number of the spawn packet or -1 after it's been ack'd
		std::int32_t spawn_pack = -1;
		// sequence number of the despawn packet or -1 if no despawn has been sent
		std::int32_t despawn_pack = -1;

		explicit SpawnStatus(Entity &);
		~SpawnStatus();
	};

private:
	void OnPacketReceived(std::uint16_t) override;
	void OnPacketLost(std::uint16_t) override;

	void On(const Packet::Login &) override;
	void On(const Packet::Part &) override;
	void On(const Packet::PlayerUpdate &) override;

	bool CanSpawn(const Entity &) const noexcept;
	bool CanDespawn(const Entity &) const noexcept;

	void SendSpawn(SpawnStatus &);
	void SendDespawn(SpawnStatus &);
	void SendUpdate(SpawnStatus &);

private:
	Server &server;
	Connection conn;
	Entity *player;
	std::list<SpawnStatus> spawns;
	unsigned int confirm_wait;
	std::uint16_t player_update_pack;
	IntervalTimer player_update_timer;

};

}

#endif
