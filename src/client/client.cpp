#include "InitialState.hpp"
#include "InteractiveState.hpp"
#include "MasterState.hpp"

#include "../app/Environment.hpp"
#include "../app/init.hpp"
#include "../app/TextureIndex.hpp"
#include "../model/CompositeModel.hpp"

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
, block_types()
, save(master.GetEnv().config.GetWorldPath(master.GetWorldConf().name, master.GetClientConf().host))
, world(block_types, master.GetWorldConf())
, interface(
	master.GetInterfaceConf(),
	master.GetEnv(),
	world,
	world.AddPlayer(master.GetInterfaceConf().player_name, player_id)
)
, chunk_renderer(*interface.GetPlayer().chunks)
, skeletons()
, update_timer(16) {
	TextureIndex tex_index;
	master.GetEnv().loader.LoadBlockTypes("default", block_types, tex_index);
	chunk_renderer.LoadTextures(master.GetEnv().loader, tex_index);
	chunk_renderer.FogDensity(master.GetWorldConf().fog_density);
	skeletons.Load();
	// TODO: better solution for initializing HUD
	interface.SelectNext();
	update_timer.Start();
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
	master.Update(dt);

	interface.Update(dt);
	world.Update(dt);
	chunk_renderer.Update(dt);

	update_timer.Update(dt);

	Entity &player = *interface.GetPlayer().entity;

	if (update_timer.Hit()) {
		master.GetClient().SendPlayerUpdate(player);
	}

	glm::mat4 trans = player.Transform(player.ChunkCoords());
	glm::vec3 dir(trans * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	glm::vec3 up(trans * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	master.GetEnv().audio.Position(player.Position());
	master.GetEnv().audio.Velocity(player.Velocity());
	master.GetEnv().audio.Orientation(dir, up);
}

void InteractiveState::Render(Viewport &viewport) {
	Entity &player = *interface.GetPlayer().entity;
	viewport.WorldPosition(player.Transform(player.ChunkCoords()));
	chunk_renderer.Render(viewport);
	world.Render(viewport);
	interface.Render(viewport);
}


MasterState::MasterState(
	Environment &env,
	const World::Config &wc,
	const Interface::Config &ic,
	const Client::Config &cc)
: env(env)
, world_conf(wc)
, intf_conf(ic)
, client_conf(cc)
, state()
, client(cc)
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
	login_packet = client.SendLogin(intf_conf.player_name);
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
		login_packet = client.SendLogin(intf_conf.player_name);
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

	pack.ReadPlayer(*state->GetInterface().GetPlayer().entity);

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
	cout << "spawned entity " << entity.Name() << " at " << entity.AbsolutePosition() << endl;
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
			cout << "despawned entity " << entity.Name() << " at " << entity.AbsolutePosition() << endl;
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
				pack.ReadEntity(*world_iter, i);
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

	int pack_diff = int16_t(seq) - int16_t(entry->second.last_packet);
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

}
}
