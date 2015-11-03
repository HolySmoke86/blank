#include "ClientConnection.hpp"
#include "ChunkTransmitter.hpp"
#include "Server.hpp"

#include "../app/init.hpp"
#include "../geometry/distance.hpp"
#include "../io/WorldSave.hpp"
#include "../model/Model.hpp"
#include "../world/ChunkIndex.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"

#include <algorithm>
#include <iostream>
#include <zlib.h>
#include <glm/gtx/io.hpp>

using namespace std;


namespace blank {
namespace server {

ChunkTransmitter::ChunkTransmitter(ClientConnection &conn)
: conn(conn)
, current(nullptr)
, buffer_size(Chunk::BlockSize() + 10)
, buffer(new uint8_t[buffer_size])
, buffer_len(0)
, packet_len(Packet::ChunkData::MAX_DATA_LEN)
, cursor(0)
, num_packets(0)
, begin_packet(-1)
, data_packets()
, confirm_wait(0)
, trans_id(0)
, compressed(false) {

}

ChunkTransmitter::~ChunkTransmitter() {
	Abort();
}

bool ChunkTransmitter::Idle() const noexcept {
	return !Transmitting() && !Waiting();
}

bool ChunkTransmitter::Transmitting() const noexcept {
	return cursor < num_packets;
}

void ChunkTransmitter::Transmit() {
	if (cursor < num_packets) {
		SendData(cursor);
		++cursor;
	}
}

bool ChunkTransmitter::Waiting() const noexcept {
	return confirm_wait > 0;
}

void ChunkTransmitter::Ack(uint16_t seq) {
	if (!Waiting()) {
		return;
	}
	if (seq == begin_packet) {
		begin_packet = -1;
		--confirm_wait;
		if (Idle()) {
			Release();
		}
		return;
	}
	for (int i = 0, end = data_packets.size(); i < end; ++i) {
		if (seq == data_packets[i]) {
			data_packets[i] = -1;
			--confirm_wait;
			if (Idle()) {
				Release();
			}
			return;
		}
	}
}

void ChunkTransmitter::Nack(uint16_t seq) {
	if (!Waiting()) {
		return;
	}
	if (seq == begin_packet) {
		SendBegin();
		return;
	}
	for (size_t i = 0, end = data_packets.size(); i < end; ++i) {
		if (seq == data_packets[i]) {
			SendData(i);
			return;
		}
	}
}

void ChunkTransmitter::Abort() {
	if (!current) return;

	Release();

	begin_packet = -1;
	data_packets.clear();
	confirm_wait = 0;
}

void ChunkTransmitter::Send(Chunk &chunk) {
	// abort current chunk, if any
	Abort();

	current = &chunk;
	current->Ref();

	// load new chunk data
	compressed = true;
	buffer_len = buffer_size;
	if (compress(buffer.get(), &buffer_len, reinterpret_cast<const Bytef *>(chunk.BlockData()), Chunk::BlockSize()) != Z_OK) {
		// compression failed, send it uncompressed
		buffer_len = Chunk::BlockSize();
		memcpy(buffer.get(), chunk.BlockData(), buffer_len);
		compressed = false;
	}
	cursor = 0;
	num_packets = (buffer_len / packet_len) + (buffer_len % packet_len != 0);
	data_packets.resize(num_packets, -1);

	++trans_id;
	SendBegin();
}

void ChunkTransmitter::SendBegin() {
	uint32_t flags = compressed;
	auto pack = conn.Prepare<Packet::ChunkBegin>();
	pack.WriteTransmissionId(trans_id);
	pack.WriteFlags(flags);
	pack.WriteChunkCoords(current->Position());
	pack.WriteDataSize(buffer_len);
	if (begin_packet == -1) {
		++confirm_wait;
	}
	begin_packet = conn.Send();
}

void ChunkTransmitter::SendData(size_t i) {
	int pos = i * packet_len;
	int len = min(packet_len, buffer_len - pos);
	const uint8_t *data = &buffer[pos];

	auto pack = conn.Prepare<Packet::ChunkData>();
	pack.WriteTransmissionId(trans_id);
	pack.WriteDataOffset(pos);
	pack.WriteDataSize(len);
	pack.WriteData(data, len);

	if (data_packets[i] == -1) {
		++confirm_wait;
	}
	data_packets[i] = conn.Send();
}

void ChunkTransmitter::Release() {
	if (current) {
		current->UnRef();
		current = nullptr;
	}
}


ClientConnection::ClientConnection(Server &server, const IPaddress &addr)
: server(server)
, conn(addr)
, input()
, player_model(nullptr)
, spawns()
, confirm_wait(0)
, entity_updates()
, entity_updates_skipped(0)
, player_update_state()
, player_update_pack(0)
, player_update_timer(1500)
, old_actions(0)
, transmitter(*this)
, chunk_queue()
, old_base() {
	conn.SetHandler(this);
}

ClientConnection::~ClientConnection() {
	DetachPlayer();
}

void ClientConnection::Update(int dt) {
	conn.Update(dt);
	if (Disconnected()) {
		return;
	}
	if (HasPlayer()) {
		// sync entities
		auto global_iter = server.GetWorld().Entities().begin();
		auto global_end = server.GetWorld().Entities().end();
		auto local_iter = spawns.begin();
		auto local_end = spawns.end();

		while (global_iter != global_end && local_iter != local_end) {
			if (global_iter->ID() == local_iter->entity->ID()) {
				// they're the same
				if (CanDespawn(*global_iter)) {
					SendDespawn(*local_iter);
				} else if (SendingUpdates()) {
					// update
					QueueUpdate(*local_iter);
				}
				++global_iter;
				++local_iter;
			} else if (global_iter->ID() < local_iter->entity->ID()) {
				// global entity was inserted
				if (CanSpawn(*global_iter)) {
					auto spawned = spawns.emplace(local_iter, *global_iter);
					SendSpawn(*spawned);
				}
				++global_iter;
			} else {
				// global entity was removed
				SendDespawn(*local_iter);
				++local_iter;
			}
		}

		// leftover spawns
		while (global_iter != global_end) {
			if (CanSpawn(*global_iter)) {
				spawns.emplace_back(*global_iter);
				SendSpawn(spawns.back());
			}
			++global_iter;
		}

		// leftover despawns
		while (local_iter != local_end) {
			SendDespawn(*local_iter);
			++local_iter;
		}
		SendUpdates();

		CheckPlayerFix();
		CheckChunkQueue();
	}
	if (conn.ShouldPing()) {
		conn.SendPing(server.GetPacket(), server.GetSocket());
	}
}

ClientConnection::SpawnStatus::SpawnStatus(Entity &e)
: entity(&e)
, spawn_pack(-1)
, despawn_pack(-1) {
	entity->Ref();
}

ClientConnection::SpawnStatus::~SpawnStatus() {
	entity->UnRef();
}

bool ClientConnection::CanSpawn(const Entity &e) const noexcept {
	return
		&e != &PlayerEntity() &&
		!e.Dead() &&
		manhattan_radius(e.ChunkCoords() - PlayerEntity().ChunkCoords()) < 7;
}

bool ClientConnection::CanDespawn(const Entity &e) const noexcept {
	return
		e.Dead() ||
		manhattan_radius(e.ChunkCoords() - PlayerEntity().ChunkCoords()) > 7;
}

uint16_t ClientConnection::Send() {
	return conn.Send(server.GetPacket(), server.GetSocket());
}

uint16_t ClientConnection::Send(size_t len) {
	server.GetPacket().len = sizeof(Packet::Header) + len;
	return Send();
}

void ClientConnection::SendSpawn(SpawnStatus &status) {
	// don't double spawn
	if (status.spawn_pack != -1) return;

	auto pack = Prepare<Packet::SpawnEntity>();
	pack.WriteEntity(*status.entity);
	status.spawn_pack = Send();
	++confirm_wait;
}

void ClientConnection::SendDespawn(SpawnStatus &status) {
	// don't double despawn
	if (status.despawn_pack != -1) return;

	auto pack = Prepare<Packet::DespawnEntity>();
	pack.WriteEntityID(status.entity->ID());
	status.despawn_pack = Send();
	++confirm_wait;
}

bool ClientConnection::SendingUpdates() const noexcept {
	return entity_updates_skipped >= NetStat().SuggestedPacketSkip();
}

void ClientConnection::QueueUpdate(SpawnStatus &status) {
	// don't send updates while spawn not ack'd or despawn sent
	if (status.spawn_pack == -1 && status.despawn_pack == -1) {
		entity_updates.push_back(&status);
	}
}

void ClientConnection::SendUpdates() {
	if (!SendingUpdates()) {
		entity_updates.clear();
		++entity_updates_skipped;
		return;
	}
	auto base = PlayerChunks().Base();
	auto pack = Prepare<Packet::EntityUpdate>();
	pack.WriteChunkBase(base);
	int entity_pos = 0;
	for (SpawnStatus *status : entity_updates) {
		pack.WriteEntity(*status->entity, base, entity_pos);
		++entity_pos;
		if (entity_pos == Packet::EntityUpdate::MAX_ENTITIES) {
			pack.WriteEntityCount(entity_pos);
			Send(Packet::EntityUpdate::GetSize(entity_pos));
			pack = Prepare<Packet::EntityUpdate>();
			entity_pos = 0;
		}
	}
	if (entity_pos > 0) {
		pack.WriteEntityCount(entity_pos);
		Send(Packet::EntityUpdate::GetSize(entity_pos));
	}
	entity_updates.clear();
	entity_updates_skipped = 0;
}

void ClientConnection::CheckPlayerFix() {
	// player_update_state's position holds the client's most recent prediction
	glm::vec3 diff = player_update_state.Diff(PlayerEntity().GetState());
	float dist_squared = dot(diff, diff);

	// if client's prediction is off by more than 1cm, send
	// our (authoritative) state back so it can fix it
	constexpr float fix_thresh = 0.0001f;

	if (dist_squared > fix_thresh) {
		auto pack = Prepare<Packet::PlayerCorrection>();
		pack.WritePacketSeq(player_update_pack);
		pack.WritePlayer(PlayerEntity());
		Send();
	}
}

namespace {

struct QueueCompare {
	explicit QueueCompare(const glm::ivec3 &base)
	: base(base) { }
	bool operator ()(const glm::ivec3 &left, const glm::ivec3 &right) const noexcept {
		const glm::ivec3 ld(left - base);
		const glm::ivec3 rd(right - base);
		return
			ld.x * ld.x + ld.y * ld.y + ld.z * ld.z <
			rd.x * rd.x + rd.y * rd.y + rd.z * rd.z;
	}
	const glm::ivec3 &base;
};

}

void ClientConnection::CheckChunkQueue() {
	if (PlayerChunks().Base() != old_base) {
		ExactLocation::Coarse begin = PlayerChunks().CoordsBegin();
		ExactLocation::Coarse end = PlayerChunks().CoordsEnd();
		for (ExactLocation::Coarse pos = begin; pos.z < end.z; ++pos.z) {
			for (pos.y = begin.y; pos.y < end.y; ++pos.y) {
				for (pos.x = begin.x; pos.x < end.x; ++pos.x) {
					if (manhattan_radius(pos - old_base) > PlayerChunks().Extent()) {
						chunk_queue.push_back(pos);
					}
				}
			}
		}
		old_base = PlayerChunks().Base();
		sort(chunk_queue.begin(), chunk_queue.end(), QueueCompare(old_base));
	}
	// if we have packet skip enabled and just pushed an entity
	// update, don't also send chunk data
	if (NetStat().SuggestedPacketSkip() > 0 && entity_updates_skipped == 0) {
		return;
	}
	if (transmitter.Transmitting()) {
		transmitter.Transmit();
		return;
	}
	if (transmitter.Idle()) {
		int count = 0;
		constexpr int max = 64;
		while (count < max && !chunk_queue.empty()) {
			ExactLocation::Coarse pos = chunk_queue.front();
			chunk_queue.pop_front();
			if (PlayerChunks().InRange(pos)) {
				Chunk *chunk = PlayerChunks().Get(pos);
				if (chunk) {
					transmitter.Send(*chunk);
					return;
				} else {
					chunk_queue.push_back(pos);
				}
				++count;
			}
		}
	}
}

void ClientConnection::AttachPlayer(Player &player) {
	DetachPlayer();
	input.reset(new DirectInput(server.GetWorld(), player, server));
	PlayerEntity().Ref();

	old_base = PlayerChunks().Base();
	ExactLocation::Coarse begin = PlayerChunks().CoordsBegin();
	ExactLocation::Coarse end = PlayerChunks().CoordsEnd();
	for (ExactLocation::Coarse pos = begin; pos.z < end.z; ++pos.z) {
		for (pos.y = begin.y; pos.y < end.y; ++pos.y) {
			for (pos.x = begin.x; pos.x < end.x; ++pos.x) {
				chunk_queue.push_back(pos);
			}
		}
	}
	sort(chunk_queue.begin(), chunk_queue.end(), QueueCompare(old_base));
	// TODO: should the server do this?
	if (HasPlayerModel()) {
		GetPlayerModel().Instantiate(PlayerEntity().GetModel());
	}

	string msg = "player \"" + player.Name() + "\" joined";
	cout << msg << endl;
	server.DistributeMessage(0, 0, msg);
}

void ClientConnection::DetachPlayer() {
	if (!HasPlayer()) return;
	string msg = "player \"" + input->GetPlayer().Name() + "\" left";
	cout << msg << endl;
	server.DistributeMessage(0, 0, msg);

	server.GetWorldSave().Write(input->GetPlayer());
	PlayerEntity().Kill();
	PlayerEntity().UnRef();
	input.reset();
	transmitter.Abort();
	chunk_queue.clear();
	old_actions = 0;
}

void ClientConnection::SetPlayerModel(const Model &m) noexcept {
	player_model = &m;
	if (HasPlayer()) {
		m.Instantiate(PlayerEntity().GetModel());
	}
}

bool ClientConnection::HasPlayerModel() const noexcept {
	return player_model;
}

const Model &ClientConnection::GetPlayerModel() const noexcept {
	return *player_model;
}

void ClientConnection::OnPacketReceived(uint16_t seq) {
	if (transmitter.Waiting()) {
		transmitter.Ack(seq);
	}
	if (!confirm_wait) return;
	for (auto iter = spawns.begin(), end = spawns.end(); iter != end; ++iter) {
		if (seq == iter->spawn_pack) {
			iter->spawn_pack = -1;
			--confirm_wait;
			return;
		}
		if (seq == iter->despawn_pack) {
			spawns.erase(iter);
			--confirm_wait;
			return;
		}
	}
}

void ClientConnection::OnPacketLost(uint16_t seq) {
	if (transmitter.Waiting()) {
		transmitter.Nack(seq);
	}
	if (!confirm_wait) return;
	for (SpawnStatus &status : spawns) {
		if (seq == status.spawn_pack) {
			status.spawn_pack = -1;
			--confirm_wait;
			SendSpawn(status);
			return;
		}
		if (seq == status.despawn_pack) {
			status.despawn_pack = -1;
			--confirm_wait;
			SendDespawn(status);
			return;
		}
	}
}

void ClientConnection::On(const Packet::Login &pack) {
	string name;
	pack.ReadPlayerName(name);

	Player *new_player = server.JoinPlayer(name);

	if (new_player) {
		// success!
		AttachPlayer(*new_player);
		cout << "accepted login from player \"" << name << '"' << endl;
		auto response = Prepare<Packet::Join>();
		response.WritePlayer(new_player->GetEntity());
		response.WriteWorldName(server.GetWorld().Name());
		Send();
		// set up update tracking
		player_update_state = new_player->GetEntity().GetState();
		player_update_pack = pack.Seq();
		player_update_timer.Reset();
		player_update_timer.Start();
	} else {
		// aw no :(
		cout << "rejected login from player \"" << name << '"' << endl;
		Prepare<Packet::Part>();
		Send();
		conn.Close();
	}
}

void ClientConnection::On(const Packet::Part &) {
	conn.Close();
}

void ClientConnection::On(const Packet::PlayerUpdate &pack) {
	if (!HasPlayer()) return;
	int pack_diff = int16_t(pack.Seq()) - int16_t(player_update_pack);
	bool overdue = player_update_timer.HitOnce();
	player_update_timer.Reset();
	if (pack_diff <= 0 && !overdue) {
		// drop old packets if we have a fairly recent state
		return;
	}
	glm::vec3 movement(0.0f);
	uint8_t new_actions;
	uint8_t slot;

	player_update_pack = pack.Seq();
	pack.ReadPredictedState(player_update_state);
	pack.ReadMovement(movement);
	pack.ReadActions(new_actions);
	pack.ReadSlot(slot);

	input->SetMovement(movement);
	input->TurnHead(player_update_state.pitch - input->GetPitch(), player_update_state.yaw - input->GetYaw());
	input->SelectInventory(slot);

	if ((new_actions & 0x01) && !(old_actions & 0x01)) {
		input->StartPrimaryAction();
	} else if (!(new_actions & 0x01) && (old_actions & 0x01)) {
		input->StopPrimaryAction();
	}
	if ((new_actions & 0x02) && !(old_actions & 0x02)) {
		input->StartSecondaryAction();
	} else if (!(new_actions & 0x02) && (old_actions & 0x02)) {
		input->StopSecondaryAction();
	}
	if ((new_actions & 0x04) && !(old_actions & 0x04)) {
		input->StartTertiaryAction();
	} else if (!(new_actions & 0x04) && (old_actions & 0x04)) {
		input->StopTertiaryAction();
	}
	old_actions = new_actions;
}

bool ClientConnection::ChunkInRange(const glm::ivec3 &pos) const noexcept {
	return HasPlayer() && PlayerChunks().InRange(pos);
}

void ClientConnection::On(const Packet::Message &pack) {
	uint8_t type;
	uint32_t ref;
	string msg;
	pack.ReadType(type);
	pack.ReadReferral(ref);
	pack.ReadMessage(msg);

	if (type == 1 && HasPlayer()) {
		server.DispatchMessage(input->GetPlayer(), msg);
	}
}


Server::Server(
	const Config::Network &conf,
	World &world,
	const World::Config &wc,
	const WorldSave &save)
: serv_sock(nullptr)
, serv_pack{ -1, nullptr, 0 }
, clients()
, world(world)
, spawn_index(world.Chunks().MakeIndex(wc.spawn, 3))
, save(save)
, player_model(nullptr)
, cli(world) {
	serv_sock = SDLNet_UDP_Open(conf.port);
	if (!serv_sock) {
		throw NetError("SDLNet_UDP_Open");
	}

	serv_pack.data = new Uint8[sizeof(Packet)];
	serv_pack.maxlen = sizeof(Packet);
}

Server::~Server() {
	world.Chunks().UnregisterIndex(spawn_index);
	delete[] serv_pack.data;
	SDLNet_UDP_Close(serv_sock);
}


void Server::Handle() {
	int result = SDLNet_UDP_Recv(serv_sock, &serv_pack);
	while (result > 0) {
		HandlePacket(serv_pack);
		result = SDLNet_UDP_Recv(serv_sock, &serv_pack);
	}
	if (result == -1) {
		// a boo boo happened
		throw NetError("SDLNet_UDP_Recv");
	}
}

void Server::HandlePacket(const UDPpacket &udp_pack) {
	if (udp_pack.len < int(sizeof(Packet::Header))) {
		// packet too small, drop
		return;
	}
	const Packet &pack = *reinterpret_cast<const Packet *>(udp_pack.data);
	if (pack.header.tag != Packet::TAG) {
		// mistagged packet, drop
		return;
	}

	ClientConnection &client = GetClient(udp_pack.address);
	client.GetConnection().Received(udp_pack);
}

ClientConnection &Server::GetClient(const IPaddress &addr) {
	for (ClientConnection &client : clients) {
		if (client.Matches(addr)) {
			return client;
		}
	}
	clients.emplace_back(*this, addr);
	if (HasPlayerModel()) {
		clients.back().SetPlayerModel(GetPlayerModel());
	}
	return clients.back();
}

void Server::Update(int dt) {
	for (list<ClientConnection>::iterator client(clients.begin()), end(clients.end()); client != end;) {
		client->Update(dt);
		if (client->Disconnected()) {
			client = clients.erase(client);
		} else {
			++client;
		}
	}
}

void Server::SetPlayerModel(const Model &m) noexcept {
	player_model = &m;
	for (ClientConnection &client : clients) {
		client.SetPlayerModel(m);
	}
}

bool Server::HasPlayerModel() const noexcept {
	return player_model;
}

const Model &Server::GetPlayerModel() const noexcept {
	return *player_model;
}

Player *Server::JoinPlayer(const string &name) {
	if (spawn_index.MissingChunks() > 0) {
		return nullptr;
	}
	Player *player = world.AddPlayer(name);
	if (!player) {
		return nullptr;
	}
	if (save.Exists(*player)) {
		save.Read(*player);
	} else {
		// TODO: spawn
	}
	return player;
}

void Server::SetBlock(Chunk &chunk, int index, const Block &block) {
	chunk.SetBlock(index, block);
	// TODO: batch chunk changes
	auto pack = Packet::Make<Packet::BlockUpdate>(GetPacket());
	pack.WriteChunkCoords(chunk.Position());
	pack.WriteBlockCount(uint32_t(1));
	pack.WriteIndex(index, 0);
	pack.WriteBlock(chunk.BlockAt(index), 0);
	GetPacket().len = sizeof(Packet::Header) + Packet::BlockUpdate::GetSize(1);
	for (ClientConnection &client : clients) {
		if (client.ChunkInRange(chunk.Position())) {
			client.Send();
		}
	}
}

void Server::DispatchMessage(Player &player, const string &msg) {
	if (msg.empty()) {
		return;
	}
	if (msg[0] == '/' && msg.size() > 1 && msg[1] != '/') {
		cli.Execute(player, msg.substr(1));
	} else {
		DistributeMessage(1, player.GetEntity().ID(), msg);
	}
}

void Server::DistributeMessage(uint8_t type, uint32_t ref, const string &msg) {
	auto pack = Packet::Make<Packet::Message>(serv_pack);
	pack.WriteType(type);
	pack.WriteReferral(ref);
	pack.WriteMessage(msg);
	serv_pack.len = sizeof(Packet::Header) + Packet::Message::GetSize(msg);
	SendAll();
}

void Server::SendAll() {
	for (ClientConnection &client : clients) {
		client.GetConnection().Send(serv_pack, serv_sock);
	}
}

}
}
