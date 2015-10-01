#include "ChunkRequester.hpp"
#include "InitialState.hpp"
#include "InteractiveState.hpp"
#include "MasterState.hpp"

#include "../app/Environment.hpp"
#include "../app/init.hpp"
#include "../app/TextureIndex.hpp"
#include "../model/CompositeModel.hpp"
#include "../io/WorldSave.hpp"
#include "../world/ChunkIndex.hpp"
#include "../world/ChunkStore.hpp"

#include <iostream>
#include <glm/gtx/io.hpp>

using namespace std;


namespace blank {
namespace client {

ChunkRequester::ChunkRequester(
	ChunkStore &store,
	const WorldSave &save
) noexcept
: store(store)
, save(save) {

}

void ChunkRequester::Update(int dt) {
	// check if there's chunks waiting to be loaded
	LoadN(10);

	// store a few chunks as well
	constexpr int max_save = 10;
	int saved = 0;
	for (Chunk &chunk : store) {
		if (chunk.ShouldUpdateSave()) {
			save.Write(chunk);
			++saved;
			if (saved >= max_save) {
				break;
			}
		}
	}
}

int ChunkRequester::ToLoad() const noexcept {
	return store.EstimateMissing();
}

void ChunkRequester::LoadOne() {
	if (!store.HasMissing()) return;

	Chunk::Pos pos = store.NextMissing();
	Chunk *chunk = store.Allocate(pos);
	if (!chunk) {
		// chunk store corrupted?
		return;
	}

	if (save.Exists(pos)) {
		save.Read(*chunk);
		// TODO: request chunk from server with cache tag
	} else {
		// TODO: request chunk from server
	}
}

void ChunkRequester::LoadN(std::size_t n) {
	std::size_t end = std::min(n, std::size_t(ToLoad()));
	for (std::size_t i = 0; i < end && store.HasMissing(); ++i) {
		LoadOne();
	}
}


InitialState::InitialState(MasterState &master)
: master(master)
, message() {
	message.Position(glm::vec3(0.0f), Gravity::CENTER);
	message.Set(master.GetEnv().assets.large_ui_font, "logging in");
}

void InitialState::OnEnter() {

}

void InitialState::Handle(const SDL_Event &evt) {
	if (evt.type == SDL_QUIT) {
		master.Quit();
	}
}

void InitialState::Update(int dt) {
	master.Update(dt);
}

void InitialState::Render(Viewport &viewport) {
	message.Render(viewport);
}


// TODO: this clutter is a giant mess
InteractiveState::InteractiveState(MasterState &master, uint32_t player_id)
: master(master)
, block_types()
, save(master.GetEnv().config.GetWorldPath(master.GetWorldConf().name, master.GetConfig().net.host))
, world(block_types, master.GetWorldConf())
, player(*world.AddPlayer(master.GetConfig().player.name))
, hud(master.GetEnv(), master.GetConfig(), player)
, manip(master.GetEnv(), player.GetEntity())
, input(world, player, master.GetClient())
, interface(master.GetConfig(), master.GetEnv().keymap, input, *this)
// TODO: looks like chunk requester and receiver can and should be merged
, chunk_requester(world.Chunks(), save)
, chunk_receiver(world.Chunks())
, chunk_renderer(player.GetChunks())
, skeletons()
, loop_timer(16)
, sky(master.GetEnv().loader.LoadCubeMap("skybox")) {
	if (!save.Exists()) {
		save.Write(master.GetWorldConf());
	}
	TextureIndex tex_index;
	master.GetEnv().loader.LoadBlockTypes("default", block_types, tex_index);
	interface.SetInventorySlots(block_types.Size() - 1);
	chunk_renderer.LoadTextures(master.GetEnv().loader, tex_index);
	chunk_renderer.FogDensity(master.GetWorldConf().fog_density);
	skeletons.Load();
	loop_timer.Start();
}

void InteractiveState::OnEnter() {
	master.GetEnv().window.GrabMouse();
}

void InteractiveState::Handle(const SDL_Event &event) {
	switch (event.type) {
		case SDL_KEYDOWN:
			interface.HandlePress(event.key);
			break;
		case SDL_KEYUP:
			interface.HandleRelease(event.key);
			break;
		case SDL_MOUSEBUTTONDOWN:
			interface.HandlePress(event.button);
			break;
		case SDL_MOUSEBUTTONUP:
			interface.HandleRelease(event.button);
			break;
		case SDL_MOUSEMOTION:
			interface.Handle(event.motion);
			break;
		case SDL_MOUSEWHEEL:
			interface.Handle(event.wheel);
			break;
		case SDL_QUIT:
			master.Quit();
			break;
		default:
			break;
	}
}

void InteractiveState::Update(int dt) {
	input.Update(dt);
	if (input.BlockFocus()) {
		hud.FocusBlock(input.BlockFocus().GetChunk(), input.BlockFocus().block);
	} else if (input.EntityFocus()) {
		hud.FocusEntity(*input.EntityFocus().entity);
	} else {
		hud.FocusNone();
	}
	hud.Display(block_types[player.GetInventorySlot() + 1]);
	loop_timer.Update(dt);
	master.Update(dt);
	chunk_receiver.Update(dt);
	chunk_requester.Update(dt);

	hud.Update(dt);
	int world_dt = 0;
	while (loop_timer.HitOnce()) {
		world.Update(loop_timer.Interval());
		world_dt += loop_timer.Interval();
		loop_timer.PopIteration();
	}
	chunk_renderer.Update(dt);

	if (world_dt > 0) {
		input.PushPlayerUpdate(world_dt);
	}

	glm::mat4 trans = player.GetEntity().Transform(player.GetEntity().ChunkCoords());
	glm::vec3 dir(trans * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	glm::vec3 up(trans * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	master.GetEnv().audio.Position(player.GetEntity().Position());
	master.GetEnv().audio.Velocity(player.GetEntity().Velocity());
	master.GetEnv().audio.Orientation(dir, up);
}

void InteractiveState::Render(Viewport &viewport) {
	viewport.WorldPosition(player.GetEntity().Transform(player.GetEntity().ChunkCoords()));
	if (master.GetConfig().video.world) {
		chunk_renderer.Render(viewport);
		world.Render(viewport);
		sky.Render(viewport);
	}
	hud.Render(viewport);
}

void InteractiveState::MergePlayerCorrection(std::uint16_t pack, const EntityState &state) {
	input.MergePlayerCorrection(pack, state);
}

void InteractiveState::Handle(const Packet::BlockUpdate &pack) {
	glm::ivec3 pos;
	pack.ReadChunkCoords(pos);
	Chunk *chunk = player.GetChunks().Get(pos);
	if (!chunk) {
		// this change doesn't concern us
		return;
	}
	uint32_t count = 0;
	pack.ReadBlockCount(count);
	for (uint32_t i = 0; i < count; ++i) {
		uint16_t index;
		Block block;
		pack.ReadIndex(index, i);
		pack.ReadBlock(block, i);
		if (index < Chunk::size && block.type < block_types.Size()) {
			manip.SetBlock(*chunk, index, block);
		}
	}
}

void InteractiveState::SetAudio(bool b) {
	master.GetConfig().audio.enabled = b;
	if (b) {
		hud.PostMessage("Audio enabled");
	} else {
		hud.PostMessage("Audio disabled");
	}
}

void InteractiveState::SetVideo(bool b) {
	master.GetConfig().video.world = b;
	if (b) {
		hud.PostMessage("World rendering enabled");
	} else {
		hud.PostMessage("World rendering disabled");
	}
}

void InteractiveState::SetHUD(bool b) {
	master.GetConfig().video.hud = b;
	if (b) {
		hud.PostMessage("HUD rendering enabled");
	} else {
		hud.PostMessage("HUD rendering disabled");
	}
}

void InteractiveState::SetDebug(bool b) {
	master.GetConfig().video.debug = b;
	if (b) {
		hud.PostMessage("Debug rendering enabled");
	} else {
		hud.PostMessage("Debug rendering disabled");
	}
}

void InteractiveState::Exit() {
	master.Quit();
}


MasterState::MasterState(
	Environment &env,
	Config &config,
	const World::Config &wc)
: env(env)
, config(config)
, world_conf(wc)
, state()
, client(config.net)
, init_state(*this)
, login_packet(-1)
, update_status()
, update_timer(16) {
	client.GetConnection().SetHandler(this);
	update_timer.Start();
}

void MasterState::Quit() {
	if (!client.GetConnection().Closed()) {
		client.SendPart();
	}
	env.state.PopUntil(this);
}


void MasterState::OnEnter() {
	login_packet = client.SendLogin(config.player.name);
	env.state.Push(&init_state);
}


void MasterState::Handle(const SDL_Event &event) {

}


void MasterState::Update(int dt) {
	update_timer.Update(dt);
	client.Handle();
	client.Update(dt);
}


void MasterState::Render(Viewport &) {

}


void MasterState::OnPacketLost(uint16_t id) {
	if (id == login_packet) {
		login_packet = client.SendLogin(config.player.name);
	}
}

void MasterState::OnTimeout() {
	if (client.GetConnection().Closed()) {
		// TODO: push disconnected message
		cout << "connection timed out" << endl;
		Quit();
	}
}

void MasterState::On(const Packet::Join &pack) {
	pack.ReadWorldName(world_conf.name);

	if (state) {
		// changing worlds
		cout << "server changing worlds to \"" << world_conf.name << '"' << endl;
	} else {
		// joining game
		cout << "joined game \"" << world_conf.name << '"' << endl;
		// server received our login
		login_packet = -1;
	}

	uint32_t player_id;
	pack.ReadPlayerID(player_id);
	state.reset(new InteractiveState(*this, player_id));

	pack.ReadPlayerState(state->GetPlayer().GetEntity().GetState());

	env.state.PopAfter(this);
	env.state.Push(state.get());
}

void MasterState::On(const Packet::Part &pack) {
	if (state) {
		// kicked
		cout << "kicked by server" << endl;
	} else {
		// join refused
		cout << "login refused by server" << endl;
	}
	Quit();
}

void MasterState::On(const Packet::SpawnEntity &pack) {
	if (!state) {
		cout << "got entity spawn before world was created" << endl;
		Quit();
		return;
	}
	uint32_t entity_id;
	pack.ReadEntityID(entity_id);
	Entity &entity = state->GetWorld().ForceAddEntity(entity_id);
	UpdateEntity(entity_id, pack.Seq());
	pack.ReadEntity(entity);
	uint32_t skel_id;
	pack.ReadSkeletonID(skel_id);
	CompositeModel *skel = state->GetSkeletons().ByID(skel_id);
	if (skel) {
		skel->Instantiate(entity.GetModel());
	}
	cout << "spawned entity #" << entity_id << "  (" << entity.Name()
		<< ") at " << entity.AbsolutePosition() << endl;
}

void MasterState::On(const Packet::DespawnEntity &pack) {
	if (!state) {
		cout << "got entity despawn before world was created" << endl;
		Quit();
		return;
	}
	uint32_t entity_id;
	pack.ReadEntityID(entity_id);
	ClearEntity(entity_id);
	for (Entity &entity : state->GetWorld().Entities()) {
		if (entity.ID() == entity_id) {
			entity.Kill();
			cout << "despawned entity #" << entity_id << " (" << entity.Name() << ") at " << entity.AbsolutePosition() << endl;
			return;
		}
	}
}

void MasterState::On(const Packet::EntityUpdate &pack) {
	if (!state) {
		cout << "got entity update before world was created" << endl;
		Quit();
		return;
	}

	auto world_iter = state->GetWorld().Entities().begin();
	auto world_end = state->GetWorld().Entities().end();

	uint32_t count = 0;
	pack.ReadEntityCount(count);

	for (uint32_t i = 0; i < count; ++i) {
		uint32_t entity_id = 0;
		pack.ReadEntityID(entity_id, i);

		while (world_iter != world_end && world_iter->ID() < entity_id) {
			++world_iter;
		}
		if (world_iter == world_end) {
			// nothing can be done from here
			return;
		}
		if (world_iter->ID() == entity_id) {
			if (UpdateEntity(entity_id, pack.Seq())) {
				pack.ReadEntityState(world_iter->GetState(), i);
			}
		}
	}
}

bool MasterState::UpdateEntity(uint32_t entity_id, uint16_t seq) {
	auto entry = update_status.find(entity_id);
	if (entry == update_status.end()) {
		update_status.emplace(entity_id, UpdateStatus{ seq, update_timer.Elapsed() });
		return true;
	}

	int16_t pack_diff = int16_t(seq) - int16_t(entry->second.last_packet);
	int time_diff = update_timer.Elapsed() - entry->second.last_update;
	entry->second.last_update = update_timer.Elapsed();

	if (pack_diff > 0 || time_diff > 1500) {
		entry->second.last_packet = seq;
		return true;
	} else {
		return false;
	}
}

void MasterState::ClearEntity(uint32_t entity_id) {
	update_status.erase(entity_id);
}

void MasterState::On(const Packet::PlayerCorrection &pack) {
	if (!state) {
		cout << "got player correction without a player :S" << endl;
		Quit();
		return;
	}
	uint16_t pack_seq;
	EntityState corrected_state;
	pack.ReadPacketSeq(pack_seq);
	pack.ReadPlayerState(corrected_state);
	state->MergePlayerCorrection(pack_seq, corrected_state);
}

void MasterState::On(const Packet::ChunkBegin &pack) {
	if (!state) {
		cout << "got chunk data, but the world has not been created yet" << endl;
		cout << "great, this will totally screw up everything :(" << endl;
		return;
	}
	state->GetChunkReceiver().Handle(pack);
}

void MasterState::On(const Packet::ChunkData &pack) {
	if (!state) {
		cout << "got chunk data, but the world has not been created yet" << endl;
		cout << "great, this will totally screw up everything :(" << endl;
		return;
	}
	state->GetChunkReceiver().Handle(pack);
}

void MasterState::On(const Packet::BlockUpdate &pack) {
	if (!state) {
		cout << "received block update, but the world has not been created yet" << endl;
		return;
	}
	state->Handle(pack);
}

}
}
