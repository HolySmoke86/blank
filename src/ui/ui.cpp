#include "ClientController.hpp"
#include "DirectInput.hpp"
#include "HUD.hpp"
#include "InteractiveManipulator.hpp"
#include "Interface.hpp"
#include "Keymap.hpp"
#include "PlayerController.hpp"

#include "../app/Assets.hpp"
#include "../app/Config.hpp"
#include "../app/Environment.hpp"
#include "../app/FrameCounter.hpp"
#include "../app/init.hpp"
#include "../audio/Audio.hpp"
#include "../audio/SoundBank.hpp"
#include "../geometry/distance.hpp"
#include "../graphics/Font.hpp"
#include "../graphics/Viewport.hpp"
#include "../io/TokenStreamReader.hpp"
#include "../model/bounds.hpp"
#include "../net/CongestionControl.hpp"
#include "../world/BlockLookup.hpp"
#include "../world/World.hpp"
#include "../world/WorldManipulator.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/io.hpp>


namespace blank {

PlayerController::PlayerController(World &world, Player &player)
: world(world)
, player(player)
, move_dir(0.0f)
, dirty(true)
, aim_world()
, aim_entity() {
	player.GetEntity().SetController(*this);
	player.GetEntity().GetSteering().SetAcceleration(5.0f);
}

PlayerController::~PlayerController() {
	if (&player.GetEntity().GetController() == this) {
		player.GetEntity().UnsetController();
	}
}

void PlayerController::SetMovement(const glm::vec3 &m) noexcept {
	if (dot(m, m) > 1.0f) {
		move_dir = normalize(m);
	} else {
		move_dir = m;
	}
	Invalidate();
}

void PlayerController::TurnHead(float dp, float dy) noexcept {
	player.GetEntity().TurnHead(dp, dy);
}

float PlayerController::GetPitch() const noexcept {
	return player.GetEntity().Pitch();
}

float PlayerController::GetYaw() const noexcept {
	return player.GetEntity().Yaw();
}

void PlayerController::SelectInventory(int i) noexcept {
	player.SetInventorySlot(i);
}

int PlayerController::InventorySlot() const noexcept {
	return player.GetInventorySlot();
}

void PlayerController::Invalidate() noexcept {
	dirty = true;
}

void PlayerController::UpdatePlayer() noexcept {
	if (dirty) {
		Ray aim = player.Aim();
		Entity &entity = player.GetEntity();
		if (!world.Intersection(aim, entity.ChunkCoords(), aim_world)) {
			aim_world = WorldCollision();
		}
		if (!world.Intersection(aim, entity, aim_entity)) {
			aim_entity = EntityCollision();
		}
		if (aim_world && aim_entity) {
			// got both, pick the closest one
			if (aim_world.depth < aim_entity.depth) {
				aim_entity = EntityCollision();
			} else {
				aim_world = WorldCollision();
			}
		}
		Steering &steering = entity.GetSteering();
		if (!iszero(move_dir)) {
			// scale input by max velocity, apply yaw, and transform to world space
			steering.SetTargetVelocity(glm::vec3(
				glm::vec4(rotateY(move_dir * entity.MaxVelocity(), entity.Yaw()), 0.0f)
				* transpose(entity.Transform())
			));
			steering.Enable(Steering::TARGET_VELOCITY);
			steering.Disable(Steering::HALT);
		} else {
			// target velocity of 0 is the same as halt
			steering.Enable(Steering::HALT);
			steering.Disable(Steering::TARGET_VELOCITY);
		}
		dirty = false;
	}
}


DirectInput::DirectInput(World &world, Player &player, WorldManipulator &manip)
: PlayerController(world, player)
, manip(manip)
, place_timer(0.25f)
, remove_timer(0.25f) {

}

void DirectInput::Update(Entity &, float dt) {
	Invalidate(); // world has changed in the meantime
	UpdatePlayer();

	remove_timer.Update(dt);
	if (remove_timer.Hit()) {
		RemoveBlock();
	}

	place_timer.Update(dt);
	if (place_timer.Hit()) {
		PlaceBlock();
	}
}

void DirectInput::StartPrimaryAction() {
	if (!remove_timer.Running()) {
		RemoveBlock();
		remove_timer.Start();
	}
}

void DirectInput::StopPrimaryAction() {
	remove_timer.Stop();
}

void DirectInput::StartSecondaryAction() {
	if (!place_timer.Running()) {
		PlaceBlock();
		place_timer.Start();
	}
}

void DirectInput::StopSecondaryAction() {
	place_timer.Stop();
}

void DirectInput::StartTertiaryAction() {
	PickBlock();
}

void DirectInput::StopTertiaryAction() {
	// nothing
}

void DirectInput::PickBlock() {
	UpdatePlayer();
	if (!BlockFocus()) return;
	SelectInventory(BlockFocus().GetBlock().type - 1);
}

void DirectInput::PlaceBlock() {
	// update block focus
	UpdatePlayer();
	// do nothing if not looking at any block
	if (!BlockFocus()) return;

	// determine block adjacent to the face the player is looking at
	BlockLookup next_block(BlockFocus().chunk, BlockFocus().BlockPos(), Block::NormalFace(BlockFocus().normal));
	// abort if it's unavailable
	if (!next_block) {
		return;
	}

	// "can replace" check
	// this prevents players from replacing solid blocks e.g. by looking through slabs
	// simple for now, should be expanded to include things like
	// entities in the way or replacable blocks like water and stuff
	if (next_block.GetBlock().type != 0) {
		return;
	}

	Block new_block(InventorySlot() + 1);

	// block's up vector
	// align with player's up
	const glm::vec3 player_up(GetPlayer().GetEntity().Up());
	new_block.SetFace(Block::NormalFace(player_up));
	// to align with player's local up/down look (like stairs in minecraft), just invert
	// it if pitch is positive
	// or, align with focus normal (like logs in minecraft)

	// determine block's turn (local rotation about up axis)
	// when aligned with player's up (first mode, and currently the only one implemented)
	// project the player's view forward onto his entity's XZ plane and
	// use the closest cardinal direction it's pointing in
	const glm::vec3 view_forward(-GetPlayer().GetEntity().ViewTransform(GetPlayer().GetEntity().ChunkCoords())[3]);
	// if view is straight up or down, this will be a null vector (NaN after normalization)
	// in that case maybe the model forward should be used?
	// the current implementation implicitly falls back to TURN_NONE which is -Z
	const glm::vec3 local_forward(normalize(view_forward - proj(view_forward, player_up)));
	// FIXME: I suspect this only works when player_up is positive Y
	if (local_forward.x > 0.707f) {
		new_block.SetTurn(Block::TURN_RIGHT);
	} else if (local_forward.z > 0.707f) {
		new_block.SetTurn(Block::TURN_AROUND);
	} else if (local_forward.x < -0.707f) {
		new_block.SetTurn(Block::TURN_LEFT);
	}
	// for mode two ("minecraft stairs") it should work the same, but I haven't properly
	// thought that through (well, that's also true about the whole face/turn thing, but oh well)
	// mode three I have absoloutely no clue. that placement would be appropriate for pipe-like
	// blocks, where turn shouldn't make a difference, but what if it does?

	manip.SetBlock(next_block.GetChunk(), next_block.GetBlockIndex(), new_block);
	Invalidate();
}

void DirectInput::RemoveBlock() {
	UpdatePlayer();
	if (!BlockFocus()) return;
	manip.SetBlock(BlockFocus().GetChunk(), BlockFocus().block, Block(0));
	Invalidate();
}


HUD::HUD(Environment &env, Config &config, const Player &player)
: env(env)
, config(config)
, player(player)
// block focus
, outline()
, outline_transform(1.0f)
, outline_visible(false)
// "inventory"
, block()
, block_buf()
, block_transform(1.0f)
, block_label()
, block_visible(false)
// debug overlay
, counter_text()
, position_text()
, orientation_text()
, block_text()
, show_block(false)
, show_entity(false)
// net stats
, bandwidth_text()
, rtt_text()
, packet_loss_text()
, show_net(false)
// message box
, messages(env.assets.small_ui_font)
, msg_timer(5000)
, msg_keep(false)
// crosshair
, crosshair() {
	const float ls = env.assets.small_ui_font.LineSkip();

	// "inventory"
	block_transform = glm::translate(block_transform, glm::vec3(50.0f, 50.0f, 0.0f));
	block_transform = glm::scale(block_transform, glm::vec3(50.0f));
	block_transform = glm::rotate(block_transform, 3.5f, glm::vec3(1.0f, 0.0f, 0.0f));
	block_transform = glm::rotate(block_transform, 0.35f, glm::vec3(0.0f, 1.0f, 0.0f));
	block_label.Position(
		glm::vec3(50.0f, 85.0f, 0.0f),
		Gravity::NORTH_WEST,
		Gravity::NORTH
	);
	block_label.Foreground(glm::vec4(1.0f));
	block_label.Background(glm::vec4(0.5f));

	// debug overlay
	counter_text.Position(glm::vec3(-25.0f, 25.0f, 0.0f), Gravity::NORTH_EAST);
	counter_text.Foreground(glm::vec4(1.0f));
	counter_text.Background(glm::vec4(0.5f));
	position_text.Position(glm::vec3(-25.0f, 25.0f + ls, 0.0f), Gravity::NORTH_EAST);
	position_text.Foreground(glm::vec4(1.0f));
	position_text.Background(glm::vec4(0.5f));
	orientation_text.Position(glm::vec3(-25.0f, 25.0f + 2 * ls, 0.0f), Gravity::NORTH_EAST);
	orientation_text.Foreground(glm::vec4(1.0f));
	orientation_text.Background(glm::vec4(0.5f));
	block_text.Position(glm::vec3(-25.0f, 25.0f + 4 * ls, 0.0f), Gravity::NORTH_EAST);
	block_text.Foreground(glm::vec4(1.0f));
	block_text.Background(glm::vec4(0.5f));
	block_text.Set(env.assets.small_ui_font, "Block: none");
	entity_text.Position(glm::vec3(-25.0f, 25.0f + 4 * ls, 0.0f), Gravity::NORTH_EAST);
	entity_text.Foreground(glm::vec4(1.0f));
	entity_text.Background(glm::vec4(0.5f));
	entity_text.Set(env.assets.small_ui_font, "Entity: none");

	// net stats
	bandwidth_text.Position(glm::vec3(-25.0f, 25.0f + 6 * ls, 0.0f), Gravity::NORTH_EAST);
	bandwidth_text.Foreground(glm::vec4(1.0f));
	bandwidth_text.Background(glm::vec4(0.5f));
	bandwidth_text.Set(env.assets.small_ui_font, "TX: 0.0KB/s RX: 0.0KB/s");
	rtt_text.Position(glm::vec3(-25.0f, 25.0f + 7 * ls, 0.0f), Gravity::NORTH_EAST);
	rtt_text.Foreground(glm::vec4(1.0f));
	rtt_text.Background(glm::vec4(0.5f));
	rtt_text.Set(env.assets.small_ui_font, "RTT: unavailable");
	packet_loss_text.Position(glm::vec3(-25.0f, 25.0f + 8 * ls, 0.0f), Gravity::NORTH_EAST);
	packet_loss_text.Foreground(glm::vec4(1.0f));
	packet_loss_text.Background(glm::vec4(0.5f));
	packet_loss_text.Set(env.assets.small_ui_font, "Packet loss: 0.0%");

	// message box
	messages.Position(glm::vec3(25.0f, -25.0f - 2 * ls, 0.0f), Gravity::SOUTH_WEST);
	messages.Foreground(glm::vec4(1.0f));
	messages.Background(glm::vec4(0.5f));

	// crosshair
	PrimitiveMesh::Buffer buf;
	buf.vertices = std::vector<glm::vec3>({
		{ -10.0f,   0.0f, 0.0f }, { 10.0f,  0.0f, 0.0f },
		{   0.0f, -10.0f, 0.0f }, {  0.0f, 10.0f, 0.0f },
	});
	buf.indices = std::vector<PrimitiveMesh::Index>({
		0, 1, 2, 3
	});
	buf.colors.resize(4, { 255, 255, 255, 255 });
	crosshair.Update(buf);
}

namespace {

PrimitiveMesh::Buffer outl_buf;

}

void HUD::FocusBlock(const Chunk &chunk, int index) {
	const Block &block = chunk.BlockAt(index);
	const BlockType &type = chunk.Type(index);
	outl_buf.Clear();
	type.OutlinePrimitiveMesh(outl_buf);
	outline.Update(outl_buf);
	outline_transform = chunk.Transform(player.GetEntity().ChunkCoords());
	outline_transform *= chunk.ToTransform(Chunk::ToPos(index), index);
	outline_transform *= glm::scale(glm::vec3(1.005f));
	outline_visible = true;
	{
		std::stringstream s;
		s << "Block: "
			<< type.label
			<< ", face: " << block.GetFace()
			<< ", turn: " << block.GetTurn();
		block_text.Set(env.assets.small_ui_font, s.str());
	}
	show_block = true;
	show_entity = false;
}

void HUD::FocusEntity(const Entity &entity) {
	{
		std::stringstream s;
		s << "Entity: " << entity.Name();
		entity_text.Set(env.assets.small_ui_font, s.str());
	}
	show_block = false;
	show_entity = true;
}

void HUD::FocusNone() {
	outline_visible = false;
	show_block = false;
	show_entity = false;
}

void HUD::DisplayNone() {
	block_visible = false;
}

void HUD::Display(const BlockType &type) {
	block_buf.Clear();
	type.FillEntityMesh(block_buf);
	block.Update(block_buf);

	block_label.Set(env.assets.small_ui_font, type.label);

	block_visible = type.visible;
}


void HUD::UpdateDebug() {
	UpdateCounter();
	UpdatePosition();
	UpdateOrientation();
}

void HUD::UpdateCounter() {
	std::stringstream s;
	s << std::setprecision(3) <<
		"avg: " << env.counter.Average().running << "ms, "
		"peak: " << env.counter.Peak().running << "ms";
	std::string text = s.str();
	counter_text.Set(env.assets.small_ui_font, text);
}

void HUD::UpdatePosition() {
	std::stringstream s;
	s << std::setprecision(3) << "pos: " << player.GetEntity().AbsolutePosition();
	position_text.Set(env.assets.small_ui_font, s.str());
}

void HUD::UpdateOrientation() {
	std::stringstream s;
	s << std::setprecision(3) << "pitch: " << glm::degrees(player.GetEntity().Pitch())
		<< ", yaw: " << glm::degrees(player.GetEntity().Yaw());
	orientation_text.Set(env.assets.small_ui_font, s.str());
}

void HUD::PostMessage(const char *msg) {
	messages.PushLine(msg);
	msg_timer.Reset();
	msg_timer.Start();
	std::cout << msg << std::endl;
}


void HUD::UpdateNetStats(const CongestionControl &stat) {
	if (!config.video.debug) return;

	std::stringstream s;
	s << std::fixed << std::setprecision(1)
		<< "TX: " << stat.Upstream()
		<< "KB/s, RX: " << stat.Downstream() << "KB/s";
	bandwidth_text.Set(env.assets.small_ui_font, s.str());

	s.str("");
	s << "RTT: " << stat.RoundTripTime() << "ms";
	rtt_text.Set(env.assets.small_ui_font, s.str());

	s.str("");
	s << "Packet loss: " << (stat.PacketLoss() * 100.0f) << "%";
	packet_loss_text.Set(env.assets.small_ui_font, s.str());

	show_net = true;
}


void HUD::Update(int dt) {
	msg_timer.Update(dt);
	if (msg_timer.HitOnce()) {
		msg_timer.Stop();
	}

	if (config.video.debug) {
		if (env.counter.Changed()) {
			UpdateCounter();
		}
		UpdatePosition();
		UpdateOrientation();
	}
}

void HUD::Render(Viewport &viewport) noexcept {
	// block focus
	if (outline_visible && config.video.world) {
		PlainColor &outline_prog = viewport.WorldColorProgram();
		outline_prog.SetM(outline_transform);
		outline.DrawLines();
	}

	// clear depth buffer so everything renders above the world
	viewport.ClearDepth();

	if (config.video.hud) {
		// "inventory"
		if (block_visible) {
			DirectionalLighting &world_prog = viewport.HUDProgram();
			world_prog.SetLightDirection({ 1.0f, 3.0f, 5.0f });
			world_prog.SetLightColor({ 1.0f, 1.0f, 1.0f });
			world_prog.SetAmbientColor({ 0.1f, 0.1f, 0.1f });
			// disable distance fog
			world_prog.SetFogDensity(0.0f);

			viewport.DisableBlending();
			world_prog.SetM(block_transform);
			block.Draw();
			block_label.Render(viewport);
		}

		// message box
		if (msg_keep || msg_timer.Running()) {
			messages.Render(viewport);
		}

		// crosshair
		PlainColor &outline_prog = viewport.HUDColorProgram();
		viewport.EnableInvertBlending();
		viewport.SetCursor(glm::vec3(0.0f), Gravity::CENTER);
		outline_prog.SetM(viewport.Cursor());
		crosshair.DrawLines();
	}

	// debug overlay
	if (config.video.debug) {
		counter_text.Render(viewport);
		position_text.Render(viewport);
		orientation_text.Render(viewport);
		if (show_block) {
			block_text.Render(viewport);
		} else if (show_entity) {
			entity_text.Render(viewport);
		}
		if (show_net) {
			bandwidth_text.Render(viewport);
			rtt_text.Render(viewport);
			packet_loss_text.Render(viewport);
		}
	}
}


InteractiveManipulator::InteractiveManipulator(Audio &audio, const SoundBank &sounds, Entity &player)
: player(player)
, audio(audio)
, sounds(sounds) {

}

void InteractiveManipulator::SetBlock(Chunk &chunk, int index, const Block &block) {
	const BlockType &old_type = chunk.Type(index);
	chunk.SetBlock(index, block);
	const BlockType &new_type = chunk.Type(index);
	glm::vec3 coords = chunk.ToSceneCoords(player.ChunkCoords(), Chunk::ToCoords(index));
	if (new_type.id == 0) {
		if (old_type.remove_sound >= 0) {
			audio.Play(sounds[old_type.remove_sound], coords);
		}
	} else {
		if (new_type.place_sound >= 0) {
			audio.Play(sounds[new_type.place_sound], coords);
		}
	}
}


Interface::Interface(
	Config &config,
	const Keymap &keymap,
	PlayerController &pc,
	ClientController &cc)
: config(config)
, keymap(keymap)
, player_ctrl(pc)
, client_ctrl(cc)
, fwd(0)
, rev(0)
, num_slots(10)
, locked(false) {

}

void Interface::Lock() {
	fwd = glm::ivec3(0);
	rev = glm::ivec3(0);
	locked = true;
}

void Interface::Unlock() {
	locked = false;
}

void Interface::HandlePress(const SDL_KeyboardEvent &event) {
	if (!config.input.keyboard) return;

	Keymap::Action action = keymap.Lookup(event);
	switch (action) {
		case Keymap::MOVE_FORWARD:
			rev.z = 1;
			UpdateMovement();
			break;
		case Keymap::MOVE_BACKWARD:
			fwd.z = 1;
			UpdateMovement();
			break;
		case Keymap::MOVE_LEFT:
			rev.x = 1;
			UpdateMovement();
			break;
		case Keymap::MOVE_RIGHT:
			fwd.x = 1;
			UpdateMovement();
			break;
		case Keymap::MOVE_UP:
			fwd.y = 1;
			UpdateMovement();
			break;
		case Keymap::MOVE_DOWN:
			rev.y = 1;
			UpdateMovement();
			break;

		case Keymap::PRIMARY:
			player_ctrl.StartPrimaryAction();
			break;
		case Keymap::SECONDARY:
			player_ctrl.StartSecondaryAction();
			break;
		case Keymap::TERTIARY:
			player_ctrl.StartTertiaryAction();
			break;

		case Keymap::INV_NEXT:
			InvRel(1);
			break;
		case Keymap::INV_PREVIOUS:
			InvRel(-1);
			break;
		case Keymap::INV_1:
		case Keymap::INV_2:
		case Keymap::INV_3:
		case Keymap::INV_4:
		case Keymap::INV_5:
		case Keymap::INV_6:
		case Keymap::INV_7:
		case Keymap::INV_8:
		case Keymap::INV_9:
		case Keymap::INV_10:
			InvAbs(action - Keymap::INV_1);
			break;

		case Keymap::EXIT:
			client_ctrl.Exit();
			break;

		case Keymap::TOGGLE_AUDIO:
			client_ctrl.SetAudio(!config.audio.enabled);
			break;
		case Keymap::TOGGLE_VIDEO:
			client_ctrl.SetVideo(!config.video.world);
			break;
		case Keymap::TOGGLE_HUD:
			client_ctrl.SetHUD(!config.video.hud);
			break;
		case Keymap::TOGGLE_DEBUG:
			client_ctrl.SetDebug(!config.video.debug);
			break;
		case Keymap::CAMERA_NEXT:
			client_ctrl.NextCamera();
			break;

		default:
			break;
	}
}

void Interface::HandleRelease(const SDL_KeyboardEvent &event) {
	if (!config.input.keyboard) return;

	switch (keymap.Lookup(event)) {
		case Keymap::MOVE_FORWARD:
			rev.z = 0;
			UpdateMovement();
			break;
		case Keymap::MOVE_BACKWARD:
			fwd.z = 0;
			UpdateMovement();
			break;
		case Keymap::MOVE_LEFT:
			rev.x = 0;
			UpdateMovement();
			break;
		case Keymap::MOVE_RIGHT:
			fwd.x = 0;
			UpdateMovement();
			break;
		case Keymap::MOVE_UP:
			fwd.y = 0;
			UpdateMovement();
			break;
		case Keymap::MOVE_DOWN:
			rev.y = 0;
			UpdateMovement();
			break;

		case Keymap::PRIMARY:
			player_ctrl.StopPrimaryAction();
			break;
		case Keymap::SECONDARY:
			player_ctrl.StopSecondaryAction();
			break;
		case Keymap::TERTIARY:
			player_ctrl.StopTertiaryAction();
			break;

		default:
			break;
	}
}

void Interface::Handle(const SDL_MouseMotionEvent &event) {
	if (locked || !config.input.mouse) return;
	player_ctrl.TurnHead(
		event.yrel * config.input.pitch_sensitivity,
		event.xrel * config.input.yaw_sensitivity);
}

void Interface::HandlePress(const SDL_MouseButtonEvent &event) {
	if (!config.input.mouse) return;

	switch (event.button) {
		case SDL_BUTTON_LEFT:
			player_ctrl.StartPrimaryAction();
			break;
		case SDL_BUTTON_RIGHT:
			player_ctrl.StartSecondaryAction();
			break;
		case SDL_BUTTON_MIDDLE:
			player_ctrl.StartTertiaryAction();
			break;
	}
}

void Interface::HandleRelease(const SDL_MouseButtonEvent &event) {
	if (!config.input.mouse) return;

	switch (event.button) {
		case SDL_BUTTON_LEFT:
			player_ctrl.StopPrimaryAction();
			break;
		case SDL_BUTTON_RIGHT:
			player_ctrl.StopSecondaryAction();
			break;
		case SDL_BUTTON_MIDDLE:
			player_ctrl.StopTertiaryAction();
			break;
	}
}


void Interface::Handle(const SDL_MouseWheelEvent &event) {
	if (!config.input.mouse) return;

	if (event.y < 0) {
		InvRel(1);
	} else if (event.y > 0) {
		InvRel(-1);
	}
}

void Interface::UpdateMovement() {
	player_ctrl.SetMovement(glm::vec3(fwd - rev));
}

void Interface::InvAbs(int s) {
	int slot = s % num_slots;
	while (slot < 0) {
		slot += num_slots;
	}
	player_ctrl.SelectInventory(slot);
}

void Interface::InvRel(int delta) {
	InvAbs(player_ctrl.GetPlayer().GetInventorySlot() + delta);
}


Keymap::Keymap()
: codemap{ NONE } {

}

void Keymap::Map(SDL_Scancode scancode, Action action) {
	if (scancode > MAX_SCANCODE) {
		throw std::runtime_error("refusing to map scancode: too damn high");
	}
	codemap[scancode] = action;
}

Keymap::Action Keymap::Lookup(SDL_Scancode scancode) const {
	if (scancode < NUM_SCANCODES) {
		return codemap[scancode];
	} else {
		return NONE;
	}
}


void Keymap::LoadDefault() {
	Map(SDL_SCANCODE_UP, MOVE_FORWARD);
	Map(SDL_SCANCODE_W, MOVE_FORWARD);
	Map(SDL_SCANCODE_DOWN, MOVE_BACKWARD);
	Map(SDL_SCANCODE_S, MOVE_BACKWARD);
	Map(SDL_SCANCODE_LEFT, MOVE_LEFT);
	Map(SDL_SCANCODE_A, MOVE_LEFT);
	Map(SDL_SCANCODE_RIGHT, MOVE_RIGHT);
	Map(SDL_SCANCODE_D, MOVE_RIGHT);
	Map(SDL_SCANCODE_SPACE, MOVE_UP);
	Map(SDL_SCANCODE_RSHIFT, MOVE_UP);
	Map(SDL_SCANCODE_LSHIFT, MOVE_DOWN);
	Map(SDL_SCANCODE_LCTRL, MOVE_DOWN);
	Map(SDL_SCANCODE_RCTRL, MOVE_DOWN);

	Map(SDL_SCANCODE_TAB, INV_NEXT);
	Map(SDL_SCANCODE_RIGHTBRACKET, INV_NEXT);
	Map(SDL_SCANCODE_LEFTBRACKET, INV_PREVIOUS);
	Map(SDL_SCANCODE_1, INV_1);
	Map(SDL_SCANCODE_2, INV_2);
	Map(SDL_SCANCODE_3, INV_3);
	Map(SDL_SCANCODE_4, INV_4);
	Map(SDL_SCANCODE_5, INV_5);
	Map(SDL_SCANCODE_6, INV_6);
	Map(SDL_SCANCODE_7, INV_7);
	Map(SDL_SCANCODE_8, INV_8);
	Map(SDL_SCANCODE_9, INV_9);
	Map(SDL_SCANCODE_0, INV_10);

	Map(SDL_SCANCODE_INSERT, SECONDARY);
	Map(SDL_SCANCODE_MENU, TERTIARY);
	Map(SDL_SCANCODE_DELETE, PRIMARY);
	Map(SDL_SCANCODE_BACKSPACE, PRIMARY);

	Map(SDL_SCANCODE_F1, TOGGLE_HUD);
	Map(SDL_SCANCODE_F2, TOGGLE_VIDEO);
	Map(SDL_SCANCODE_F3, TOGGLE_DEBUG);
	Map(SDL_SCANCODE_F4, TOGGLE_AUDIO);
	Map(SDL_SCANCODE_F5, CAMERA_NEXT);

	Map(SDL_SCANCODE_ESCAPE, EXIT);
}


void Keymap::Load(std::istream &is) {
	TokenStreamReader in(is);
	std::string key_name;
	std::string action_name;
	SDL_Scancode key;
	Action action;
	while (in.HasMore()) {
		if (in.Peek().type == Token::STRING) {
			in.ReadString(key_name);
			key = SDL_GetScancodeFromName(key_name.c_str());
		} else {
			key = SDL_Scancode(in.GetInt());
		}
		in.Skip(Token::EQUALS);
		in.ReadIdentifier(action_name);
		action = StringToAction(action_name);
		if (in.HasMore() && in.Peek().type == Token::SEMICOLON) {
			in.Skip(Token::SEMICOLON);
		}
		Map(key, action);
	}
}

void Keymap::Save(std::ostream &out) {
	for (unsigned int i = 0; i < NUM_SCANCODES; ++i) {
		if (codemap[i] == NONE) continue;

		const char *str = SDL_GetScancodeName(SDL_Scancode(i));
		if (str && *str) {
			out << '"';
			while (*str) {
				if (*str == '"') {
					out << "\\\"";
				} else {
					out << *str;
				}
				++str;
			}
			out << '"';
		} else {
			out << i;
		}

		out << " = " << ActionToString(codemap[i]) << std::endl;;
	}
}


namespace {

std::map<std::string, Keymap::Action> action_map = {
	{ "none", Keymap::NONE },
	{ "move_forward", Keymap::MOVE_FORWARD },
	{ "move_backward", Keymap::MOVE_BACKWARD },
	{ "move_left", Keymap::MOVE_LEFT },
	{ "move_right", Keymap::MOVE_RIGHT },
	{ "move_up", Keymap::MOVE_UP },
	{ "move_down", Keymap::MOVE_DOWN },

	{ "primary", Keymap::PRIMARY },
	{ "secondary", Keymap::SECONDARY },
	{ "tertiary", Keymap::TERTIARY },

	{ "inventory_next", Keymap::INV_NEXT },
	{ "inventory_prev", Keymap::INV_PREVIOUS },
	{ "inventory_1", Keymap::INV_1 },
	{ "inventory_2", Keymap::INV_2 },
	{ "inventory_3", Keymap::INV_3 },
	{ "inventory_4", Keymap::INV_4 },
	{ "inventory_5", Keymap::INV_5 },
	{ "inventory_6", Keymap::INV_6 },
	{ "inventory_7", Keymap::INV_7 },
	{ "inventory_8", Keymap::INV_8 },
	{ "inventory_9", Keymap::INV_9 },
	{ "inventory_10", Keymap::INV_10 },

	{ "toggle_audio", Keymap::TOGGLE_AUDIO },
	{ "toggle_video", Keymap::TOGGLE_VIDEO },
	{ "toggle_hud", Keymap::TOGGLE_HUD },
	{ "toggle_debug", Keymap::TOGGLE_DEBUG },
	{ "camera_next", Keymap::CAMERA_NEXT },

	{ "exit", Keymap::EXIT },
};

}

const char *Keymap::ActionToString(Action action) {
	for (const auto &entry : action_map) {
		if (action == entry.second) {
			return entry.first.c_str();
		}
	}
	return "none";
}

Keymap::Action Keymap::StringToAction(const std::string &str) {
	auto entry = action_map.find(str);
	if (entry != action_map.end()) {
		return entry->second;
	} else {
		std::cerr << "unknown action \"" << str << '"' << std::endl;
		return NONE;
	}
}

}
