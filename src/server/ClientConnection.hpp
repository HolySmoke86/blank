#ifndef BLANK_SERVER_CLIENTCONNECTION_HPP_
#define BLANK_SERVER_CLIENTCONNECTION_HPP_

#include "ChunkTransmitter.hpp"
#include "Server.hpp"
#include "../app/IntervalTimer.hpp"
#include "../ui/DirectInput.hpp"
#include "../net/Connection.hpp"
#include "../net/ConnectionHandler.hpp"
#include "../world/EntityState.hpp"
#include "../world/Player.hpp"

#include <deque>
#include <list>
#include <memory>
#include <SDL_net.h>
#include <vector>


namespace blank {

class Model;

namespace server {

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

	/// prepare a packet of given type
	template<class Type>
	Type Prepare() const noexcept {
		return Packet::Make<Type>(server.GetPacket());
	}
	/// send the previously prepared packet
	std::uint16_t Send();
	/// send the previously prepared packet of given payload length
	std::uint16_t Send(std::size_t len);

	void AttachPlayer(Player &);
	void DetachPlayer();
	bool HasPlayer() const noexcept { return !!input; }
	Entity &PlayerEntity() noexcept { return input->GetPlayer().GetEntity(); }
	const Entity &PlayerEntity() const noexcept { return input->GetPlayer().GetEntity(); }
	ChunkIndex &PlayerChunks() noexcept { return input->GetPlayer().GetChunks(); }
	const ChunkIndex &PlayerChunks() const noexcept { return input->GetPlayer().GetChunks(); }

	void SetPlayerModel(const Model &) noexcept;
	bool HasPlayerModel() const noexcept;
	const Model &GetPlayerModel() const noexcept;

	bool ChunkInRange(const glm::ivec3 &) const noexcept;

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
	void On(const Packet::Message &) override;

	void CheckEntities();
	bool CanSpawn(const Entity &) const noexcept;
	bool CanDespawn(const Entity &) const noexcept;

	void SendSpawn(SpawnStatus &);
	void SendDespawn(SpawnStatus &);
	/// true if entity updates are pushed to the client this frame
	bool SendingUpdates() const noexcept;
	void QueueUpdate(SpawnStatus &);
	void SendUpdates();

	void CheckPlayerFix();

	void CheckChunkQueue();

private:
	Server &server;
	Connection conn;
	std::unique_ptr<DirectInput> input;
	const Model *player_model;
	std::list<SpawnStatus> spawns;
	unsigned int confirm_wait;

	std::vector<SpawnStatus *> entity_updates;
	unsigned int entity_updates_skipped;

	EntityState player_update_state;
	std::uint16_t player_update_pack;
	CoarseTimer player_update_timer;
	std::uint8_t old_actions;

	ChunkTransmitter transmitter;
	std::deque<glm::ivec3> chunk_queue;
	glm::ivec3 old_base;
	unsigned int chunk_blocks_skipped;

};

}
}

#endif
