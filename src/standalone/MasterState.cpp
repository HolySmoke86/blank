#include "MasterState.hpp"

#include "../app/Config.hpp"
#include "../app/Environment.hpp"
#include "../app/init.hpp"
#include "../app/TextureIndex.hpp"
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
, shapes()
, block_types()
, save(save)
, world(block_types, wc)
, spawn_index(world.Chunks().MakeIndex(wc.spawn, 3))
, player(*world.AddPlayer(config.player.name))
, spawn_player(false)
, hud(env, config, player)
, manip(env, player.GetEntity())
, input(world, player, manip)
, interface(config, env.keymap, input, *this)
, generator(gc)
, chunk_loader(world.Chunks(), generator, save)
, chunk_renderer(player.GetChunks())
, skeletons()
, spawner(world, skeletons, env.rng)
, sky(env.loader.LoadCubeMap("skybox"))
, preload(env, chunk_loader, chunk_renderer)
, unload(env, world.Chunks(), save) {
	TextureIndex tex_index;
	env.loader.LoadShapes("default", shapes);
	env.loader.LoadBlockTypes("default", block_types, tex_index);
	interface.SetInventorySlots(block_types.size() - 1);
	generator.LoadTypes(block_types);
	chunk_renderer.LoadTextures(env.loader, tex_index);
	chunk_renderer.FogDensity(wc.fog_density);
	skeletons.Load();
	spawner.LimitSkeletons(0, skeletons.size());
	if (save.Exists(player)) {
		save.Read(player);
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
	}
	if (config.input.mouse) {
		env.window.GrabMouse();
	}
	if (spawn_player) {
		// TODO: spawn
		spawn_player = false;
	}
}

void MasterState::OnPause() {
	env.window.ReleaseMouse();
}


void MasterState::Handle(const SDL_Event &event) {
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
			Exit();
			break;
		default:
			break;
	}
}

void MasterState::Update(int dt) {
	input.Update(dt);
	if (input.BlockFocus()) {
		hud.FocusBlock(input.BlockFocus().GetChunk(), input.BlockFocus().block);
	} else if (input.EntityFocus()) {
		hud.FocusEntity(*input.EntityFocus().entity);
	} else {
		hud.FocusNone();
	}
	hud.Display(block_types[player.GetInventorySlot() + 1]);
	hud.Update(dt);
	spawner.Update(dt);
	world.Update(dt);
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
	viewport.WorldPosition(player.GetEntity().Transform(player.GetEntity().ChunkCoords()));
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

}
}
