#include "InitialState.hpp"
#include "InteractiveState.hpp"
#include "MasterState.hpp"

#include "../app/Environment.hpp"
#include "../app/init.hpp"
#include "../app/TextureIndex.hpp"

#include <iostream>


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


InteractiveState::InteractiveState(MasterState &master)
: master(master)
, block_types()
, save(master.GetEnv().config.GetWorldPath(master.GetWorldConf().name, master.GetClientConf().host))
, world(block_types, master.GetWorldConf(), save)
, chunk_renderer(world, master.GetWorldConf().load.load_dist)
, interface(master.GetInterfaceConf(), master.GetEnv(), world) {
	TextureIndex tex_index;
	master.GetEnv().loader.LoadBlockTypes("default", block_types, tex_index);
	chunk_renderer.LoadTextures(master.GetEnv().loader, tex_index);
	chunk_renderer.FogDensity(master.GetWorldConf().fog_density);
	// TODO: better solution for initializing HUD
	interface.SelectNext();
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
	chunk_renderer.Rebase(interface.Player().ChunkCoords());
	chunk_renderer.Update(dt);

	glm::mat4 trans = interface.Player().Transform(interface.Player().ChunkCoords());
	glm::vec3 dir(trans * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	glm::vec3 up(trans * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	master.GetEnv().audio.Position(interface.Player().Position());
	master.GetEnv().audio.Velocity(interface.Player().Velocity());
	master.GetEnv().audio.Orientation(dir, up);
}

void InteractiveState::Render(Viewport &viewport) {
	viewport.WorldPosition(interface.Player().Transform(interface.Player().ChunkCoords()));
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
, login_packet(-1) {
	client.GetConnection().SetHandler(this);
}

void MasterState::Quit() {
	env.state.PopUntil(this);
}


void MasterState::OnEnter() {
	login_packet = client.SendLogin(intf_conf.player_name);
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


void MasterState::OnPacketLost(std::uint16_t id) {
	if (id == login_packet) {
		login_packet = client.SendLogin(intf_conf.player_name);
	}
}

void MasterState::OnTimeout() {
	if (client.GetConnection().Closed()) {
		Quit();
		// TODO: push disconnected message
	}
}

void MasterState::On(const Packet::Join &pack) {
	pack.ReadWorldName(world_conf.name);

	if (state) {
		// changing worlds
		std::cout << "server changing worlds" << std::endl;
	} else {
		// joining game
		std::cout << "joined game" << std::endl;
	}
	state.reset(new InteractiveState(*this));

	pack.ReadPlayer(state->GetInterface().Player());

	env.state.PopAfter(this);
	env.state.Push(state.get());
}

void MasterState::On(const Packet::Part &pack) {
	if (state) {
		// kicked
		std::cout << "kicked by server" << std::endl;
	} else {
		// join refused
		std::cout << "login refused by server" << std::endl;
	}
	Quit();
}

}
}
