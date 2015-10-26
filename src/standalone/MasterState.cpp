#include "MasterState.hpp"

#include "../app/Config.hpp"
#include "../app/Environment.hpp"
#include "../app/init.hpp"
#include "../io/WorldSave.hpp"

#include <SDL.h>


namespace blank {
namespace standalone {

MasterState::MasterState(
	Environment &env,
	Config &config,
	const Generator::Config &gc,
	const World::Config &wc,
	const WorldSave &save
)
: config(config)
, env(env)
, res()
, sounds()
, save(save)
, world(res.block_types, wc)
, spawn_index(world.Chunks().MakeIndex(wc.spawn, 3))
, player(*world.AddPlayer(config.player.name))
, spawn_player(false)
, hud(env, config, player)
, manip(env.audio, sounds, player.GetEntity())
, input(world, player, manip)
, interface(config, env.keymap, input, *this)
, generator(gc)
, chunk_loader(world.Chunks(), generator, save)
, chunk_renderer(player.GetChunks())
, spawner(world, res.models, env.rng)
, sky(env.loader.LoadCubeMap("skybox"))
, cli(world)
, preload(env, chunk_loader, chunk_renderer)
, unload(env, world.Chunks(), save)
, chat(env, *this, *this) {
	res.Load(env.loader, "default");
	if (res.models.size() < 2) {
		throw std::runtime_error("need at least two models to run");
	}
	res.models[0].Instantiate(player.GetEntity().GetModel());
	sounds.Load(env.loader, res.snd_index);
	spawner.LimitModels(1, res.models.size());
	interface.SetInventorySlots(res.block_types.size() - 1);
	generator.LoadTypes(res.block_types);
	chunk_renderer.LoadTextures(env.loader, res.tex_index);
	chunk_renderer.FogDensity(wc.fog_density);
	if (save.Exists(player)) {
		save.Read(player);
		glm::vec3 orient(glm::eulerAngles(player.GetEntity().Orientation()));
		input.TurnHead(orient.x, orient.y);
	} else {
		spawn_player = true;
	}
}

MasterState::~MasterState() {
	world.Chunks().UnregisterIndex(spawn_index);
}


void MasterState::OnResume() {
	if (spawn_index.MissingChunks() > 0) {
		env.state.Push(&preload);
		return;
	}
	if (spawn_player) {
		// TODO: spawn
		spawn_player = false;
	}
	hud.KeepMessages(false);
	OnFocus();
}

void MasterState::OnPause() {
	OnBlur();
}

void MasterState::OnFocus() {
	if (config.input.mouse) {
		env.window.GrabMouse();
	}
	interface.Unlock();
}

void MasterState::OnBlur() {
	env.window.ReleaseMouse();
	interface.Lock();
}


void MasterState::Handle(const SDL_Event &event) {
	switch (event.type) {
		case SDL_KEYDOWN:
			// TODO: move to interface
			if (event.key.keysym.sym == SDLK_RETURN) {
				chat.Clear();
				env.state.Push(&chat);
				hud.KeepMessages(true);
			} else if (event.key.keysym.sym == SDLK_SLASH) {
				chat.Preset("/");
				env.state.Push(&chat);
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

void MasterState::Update(int dt) {
	spawner.Update(dt);
	world.Update(dt);
	if (input.BlockFocus()) {
		hud.FocusBlock(input.BlockFocus().GetChunk(), input.BlockFocus().block);
	} else if (input.EntityFocus()) {
		hud.FocusEntity(*input.EntityFocus().entity);
	} else {
		hud.FocusNone();
	}
	hud.Display(res.block_types[player.GetInventorySlot() + 1]);
	hud.Update(dt);
	chunk_loader.Update(dt);
	chunk_renderer.Update(dt);

	glm::mat4 trans = player.GetEntity().Transform(player.GetEntity().ChunkCoords());
	glm::vec3 dir(trans * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	glm::vec3 up(trans * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
	env.audio.Position(player.GetEntity().Position());
	env.audio.Velocity(player.GetEntity().Velocity());
	env.audio.Orientation(dir, up);
}

void MasterState::Render(Viewport &viewport) {
	viewport.WorldPosition(
		player.GetEntity().Transform(player.GetEntity().ChunkCoords())
		* player.GetEntity().GetModel().EyesTransform());
	if (config.video.world) {
		chunk_renderer.Render(viewport);
		world.Render(viewport);
		sky.Render(viewport);
	}
	hud.Render(viewport);
}


void MasterState::SetAudio(bool b) {
	config.audio.enabled = b;
	if (b) {
		hud.PostMessage("Audio enabled");
	} else {
		hud.PostMessage("Audio disabled");
	}
}

void MasterState::SetVideo(bool b) {
	config.video.world = b;
	if (b) {
		hud.PostMessage("World rendering enabled");
	} else {
		hud.PostMessage("World rendering disabled");
	}
}

void MasterState::SetHUD(bool b) {
	config.video.hud = b;
	if (b) {
		hud.PostMessage("HUD rendering enabled");
	} else {
		hud.PostMessage("HUD rendering disabled");
	}
}

void MasterState::SetDebug(bool b) {
	config.video.debug = b;
	if (b) {
		hud.PostMessage("Debug rendering enabled");
	} else {
		hud.PostMessage("Debug rendering disabled");
	}
}

void MasterState::Exit() {
	save.Write(player);
	env.state.Switch(&unload);
}

void MasterState::OnLineSubmit(const std::string &line) {
	if (line.empty()) {
		return;
	}
	if (line[0] == '/' && line.size() > 1 && line[1] != '/') {
		cli.Execute(player, line.substr(1));
	} else {
		hud.PostMessage(line);
	}
}

}
}
