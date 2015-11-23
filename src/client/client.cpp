#include "InitialState.hpp"
#include "InteractiveState.hpp"
#include "MasterState.hpp"

#include "../app/Environment.hpp"
#include "../app/init.hpp"
#include "../geometry/distance.hpp"
#include "../model/Model.hpp"
#include "../io/WorldSave.hpp"
#include "../world/ChunkIndex.hpp"
#include "../world/ChunkStore.hpp"

#include <iostream>
#include <glm/gtx/io.hpp>

using namespace std;


namespace blank {
namespace client {

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
, res()
, sounds()
, save(master.GetEnv().config.GetWorldPath(master.GetWorldConf().name, master.GetConfig().net.host))
, world(res.block_types, master.GetWorldConf())
, player(*world.AddPlayer(master.GetConfig().player.name, player_id))
, hud(master.GetEnv(), master.GetConfig(), player)
, manip(master.GetEnv().audio, sounds, player.GetEntity())
, input(world, player, master.GetClient())
, interface(master.GetConfig(), master.GetEnv().keymap, input, *this)
, chunk_receiver(master.GetClient(), world.Chunks(), save)
, chunk_renderer(player.GetChunks())
, loop_timer(16)
, stat_timer(1000)
, sky(master.GetEnv().loader.LoadCubeMap("skybox"))
, update_status()
, chat(master.GetEnv(), *this, *this)
, time_skipped(0)
, packets_skipped(0) {
	if (!save.Exists()) {
		save.Write(master.GetWorldConf());
	}
	res.Load(master.GetEnv().loader, "default");
	if (res.models.size() < 1) {
		throw std::runtime_error("need at least one model to run");
	}
	res.models[0].Instantiate(player.GetEntity().GetModel());
	sounds.Load(master.GetEnv().loader, res.snd_index);
	interface.SetInventorySlots(res.block_types.size() - 1);
	chunk_renderer.LoadTextures(master.GetEnv().loader, res.tex_index);
	chunk_renderer.FogDensity(master.GetWorldConf().fog_density);
	loop_timer.Start();
	stat_timer.Start();
}

void InteractiveState::OnResume() {
	OnFocus();
}

void InteractiveState::OnPause() {
	OnBlur();
}

void InteractiveState::OnFocus() {
	if (master.GetConfig().input.mouse) {
		master.GetEnv().window.GrabMouse();
	}
	interface.Unlock();
}

void InteractiveState::OnBlur() {
	master.GetEnv().window.ReleaseMouse();
	interface.Lock();
}

void InteractiveState::Handle(const SDL_Event &event) {
	switch (event.type) {
		case SDL_KEYDOWN:
			// TODO: move to interface
			if (event.key.keysym.sym == SDLK_RETURN) {
				chat.Clear();
				master.GetEnv().state.Push(&chat);
				hud.KeepMessages(true);
			} else if (event.key.keysym.sym == SDLK_SLASH) {
				chat.Preset("/");
				master.GetEnv().state.Push(&chat);
				hud.KeepMessages(true);
			} else {
				interface.HandlePress(event.key);
			}
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
			Exit();
			break;
		default:
			break;
	}
}

void InteractiveState::Update(int dt) {
	loop_timer.Update(dt);
	stat_timer.Update(dt);
	master.Update(dt);
	chunk_receiver.Update(dt);

	int world_dt = 0;
	while (loop_timer.HitOnce()) {
		world.Update(loop_timer.Interval());
		world_dt += loop_timer.Interval();
		loop_timer.PopIteration();
	}
	chunk_renderer.Update(dt);

	if (input.BlockFocus()) {
		hud.FocusBlock(input.BlockFocus().GetChunk(), input.BlockFocus().block);
	} else if (input.EntityFocus()) {
		hud.FocusEntity(*input.EntityFocus().entity);
	} else {
		hud.FocusNone();
	}
	if (world_dt > 0) {
		if (input.UpdateImportant() || packets_skipped >= master.NetStat().SuggestedPacketSkip()) {
			input.PushPlayerUpdate(time_skipped + world_dt);
			time_skipped = 0;
			packets_skipped = 0;
		} else {
			time_skipped += world_dt;
			++packets_skipped;
		}
	}
	hud.Display(res.block_types[player.GetInventorySlot() + 1]);
	if (stat_timer.Hit()) {
		hud.UpdateNetStats(master.NetStat());
	}
	hud.Update(dt);

	glm::mat4 trans = player.GetEntity().Transform(player.GetEntity().ChunkCoords());
	glm::vec3 dir(trans * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	glm::vec3 up(trans * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	master.GetEnv().audio.Position(player.GetEntity().Position());
	master.GetEnv().audio.Velocity(player.GetEntity().Velocity());
	master.GetEnv().audio.Orientation(dir, up);
}

void InteractiveState::Render(Viewport &viewport) {
	viewport.WorldPosition(player.GetEntity().ViewTransform(player.GetEntity().ChunkCoords()));
	if (master.GetConfig().video.world) {
		chunk_renderer.Render(viewport);
		world.Render(viewport);
		if (master.GetConfig().video.debug) {
			world.RenderDebug(viewport);
		}
		sky.Render(viewport);
	}
	hud.Render(viewport);
}

void InteractiveState::Handle(const Packet::SpawnEntity &pack) {
	uint32_t entity_id;
	pack.ReadEntityID(entity_id);
	Entity &entity = world.ForceAddEntity(entity_id);
	UpdateEntity(entity_id, pack.Seq());
	pack.ReadEntity(entity);
	uint32_t model_id;
	pack.ReadModelID(model_id);
	if (model_id > 0 && model_id <= res.models.size()) {
		res.models.Get(model_id).Instantiate(entity.GetModel());
	}
}

void InteractiveState::Handle(const Packet::DespawnEntity &pack) {
	uint32_t entity_id;
	pack.ReadEntityID(entity_id);
	ClearEntity(entity_id);
	for (Entity &entity : world.Entities()) {
		if (entity.ID() == entity_id) {
			entity.Kill();
			return;
		}
	}
}

void InteractiveState::Handle(const Packet::EntityUpdate &pack) {
	auto world_iter = world.Entities().begin();
	auto world_end = world.Entities().end();

	uint32_t count = 0;
	glm::ivec3 base;
	pack.ReadEntityCount(count);
	pack.ReadChunkBase(base);
	EntityState state;

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
				pack.ReadEntityState(state, base, i);
				world_iter->SetState(state);
			}
		}
	}
}

bool InteractiveState::UpdateEntity(uint32_t entity_id, uint16_t seq) {
	auto entry = update_status.find(entity_id);
	if (entry == update_status.end()) {
		update_status.emplace(entity_id, UpdateStatus{ seq, loop_timer.Elapsed() });
		return true;
	}

	int16_t pack_diff = int16_t(seq) - int16_t(entry->second.last_packet);
	int time_diff = loop_timer.Elapsed() - entry->second.last_update;
	entry->second.last_update = loop_timer.Elapsed();

	if (pack_diff > 0 || time_diff > 1500) {
		entry->second.last_packet = seq;
		return true;
	} else {
		return false;
	}
}

void InteractiveState::ClearEntity(uint32_t entity_id) {
	update_status.erase(entity_id);
}

void InteractiveState::Handle(const Packet::PlayerCorrection &pack) {
	uint16_t pack_seq;
	EntityState corrected_state;
	pack.ReadPacketSeq(pack_seq);
	pack.ReadPlayerState(corrected_state);
	input.MergePlayerCorrection(pack_seq, corrected_state);
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
		if (index < Chunk::size && block.type < res.block_types.size()) {
			manip.SetBlock(*chunk, index, block);
		}
	}
}

void InteractiveState::Handle(const Packet::Message &pack) {
	string msg;
	pack.ReadMessage(msg);
	hud.PostMessage(msg);
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

void InteractiveState::NextCamera() {
	if (iszero(master.GetEnv().viewport.CameraOffset())) {
		master.GetEnv().viewport.OffsetCamera(glm::vec3(0.0f, 0.0f, -5.0f));
	} else {
		master.GetEnv().viewport.OffsetCamera(glm::vec3(0.0f, 0.0f, 0.0f));
	}
}

void InteractiveState::Exit() {
	save.Write(player);
	master.Quit();
}

void InteractiveState::OnLineSubmit(const string &line) {
	if (!line.empty()) {
		master.GetClient().SendMessage(1, 0, line);
	}
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
, login_packet(-1) {
	client.GetConnection().SetHandler(this);
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
		Quit();
		env.ShowMessage("connection timed out");
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

	EntityState player_state;
	pack.ReadPlayerState(player_state);
	state->GetPlayer().GetEntity().SetState(player_state);

	env.state.PopAfter(this);
	env.state.Push(state.get());
}

void MasterState::On(const Packet::Part &pack) {
	Quit();
	if (state) {
		// kicked
		env.ShowMessage("kicked by server");
	} else {
		// join refused
		env.ShowMessage("login refused by server");
	}
}

void MasterState::On(const Packet::SpawnEntity &pack) {
	if (!state) {
		cout << "got entity spawn before world was created" << endl;
		return;
	}
	state->Handle(pack);
}

void MasterState::On(const Packet::DespawnEntity &pack) {
	if (!state) {
		cout << "got entity despawn before world was created" << endl;
		return;
	}
	state->Handle(pack);
}

void MasterState::On(const Packet::EntityUpdate &pack) {
	if (!state) {
		cout << "got entity update before world was created" << endl;
		return;
	}
	state->Handle(pack);
}

void MasterState::On(const Packet::PlayerCorrection &pack) {
	if (!state) {
		cout << "got player correction without a player :S" << endl;
		return;
	}
	state->Handle(pack);
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

void MasterState::On(const Packet::Message &pack) {
	if (state) {
		state->Handle(pack);
	} else {
		string msg;
		pack.ReadMessage(msg);
		cout << "got message before interface was created: " << msg << endl;
	}
}

}
}
