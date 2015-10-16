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
#include "../graphics/Font.hpp"
#include "../graphics/Viewport.hpp"
#include "../io/TokenStreamReader.hpp"
#include "../model/bounds.hpp"
#include "../world/BlockLookup.hpp"
#include "../world/World.hpp"
#include "../world/WorldManipulator.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/io.hpp>


namespace blank {

PlayerController::PlayerController(World &world, Player &player)
: world(world)
, player(player)
, move_dir(0.0f)
, pitch(0.0f)
, yaw(0.0f)
, dirty(true)
, aim_world()
, aim_entity() {

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
	pitch += dp;
	if (pitch > PI / 2) {
		pitch = PI / 2;
	} else if (pitch < -PI / 2) {
		pitch = -PI / 2;
	}
	yaw += dy;
	if (yaw > PI) {
		yaw -= PI * 2;
	} else if (yaw < -PI) {
		yaw += PI * 2;
	}
	Invalidate();
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
	constexpr float max_vel = 0.005f;
	if (dirty) {
		player.GetEntity().Orientation(glm::quat(glm::vec3(pitch, yaw, 0.0f)));
		player.GetEntity().TargetVelocity(glm::rotateY(move_dir * max_vel, yaw));

		Ray aim = player.Aim();
		if (!world.Intersection(aim, glm::mat4(1.0f), player.GetEntity().ChunkCoords(), aim_world)) {
			aim_world = WorldCollision();
		}
		if (!world.Intersection(aim, glm::mat4(1.0f), player.GetEntity(), aim_entity)) {
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
		dirty = false;
	}
}


DirectInput::DirectInput(World &world, Player &player, WorldManipulator &manip)
: PlayerController(world, player)
, manip(manip)
, place_timer(256)
, remove_timer(256) {

}

void DirectInput::Update(int dt) {
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
	UpdatePlayer();
	if (!BlockFocus()) return;

	BlockLookup next_block(BlockFocus().chunk, BlockFocus().BlockPos(), Block::NormalFace(BlockFocus().normal));
	if (!next_block) {
		return;
	}
	manip.SetBlock(next_block.GetChunk(), next_block.GetBlockIndex(), Block(InventorySlot() + 1));
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
// message box
, messages(env.assets.small_ui_font)
, msg_timer(5000)
// crosshair
, crosshair() {
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
	position_text.Position(glm::vec3(-25.0f, 25.0f + env.assets.small_ui_font.LineSkip(), 0.0f), Gravity::NORTH_EAST);
	position_text.Foreground(glm::vec4(1.0f));
	position_text.Background(glm::vec4(0.5f));
	orientation_text.Position(glm::vec3(-25.0f, 25.0f + 2 * env.assets.small_ui_font.LineSkip(), 0.0f), Gravity::NORTH_EAST);
	orientation_text.Foreground(glm::vec4(1.0f));
	orientation_text.Background(glm::vec4(0.5f));
	block_text.Position(glm::vec3(-25.0f, 25.0f + 4 * env.assets.small_ui_font.LineSkip(), 0.0f), Gravity::NORTH_EAST);
	block_text.Foreground(glm::vec4(1.0f));
	block_text.Background(glm::vec4(0.5f));
	block_text.Set(env.assets.small_ui_font, "Block: none");
	entity_text.Position(glm::vec3(-25.0f, 25.0f + 4 * env.assets.small_ui_font.LineSkip(), 0.0f), Gravity::NORTH_EAST);
	entity_text.Foreground(glm::vec4(1.0f));
	entity_text.Background(glm::vec4(0.5f));
	entity_text.Set(env.assets.small_ui_font, "Entity: none");

	// message box
	messages.Position(glm::vec3(25.0f, -25.0f, 0.0f), Gravity::SOUTH_WEST);
	messages.Foreground(glm::vec4(1.0f));
	messages.Background(glm::vec4(0.5f));

	// crosshair
	OutlineMesh::Buffer buf;
	buf.vertices = std::vector<glm::vec3>({
		{ -10.0f,   0.0f, 0.0f }, { 10.0f,  0.0f, 0.0f },
		{   0.0f, -10.0f, 0.0f }, {  0.0f, 10.0f, 0.0f },
	});
	buf.indices = std::vector<OutlineMesh::Index>({
		0, 1, 2, 3
	});
	buf.colors.resize(4, { 10.0f, 10.0f, 10.0f });
	crosshair.Update(buf);
}

namespace {

OutlineMesh::Buffer outl_buf;

}

void HUD::FocusBlock(const Chunk &chunk, int index) {
	const Block &block = chunk.BlockAt(index);
	const BlockType &type = chunk.Type(index);
	outl_buf.Clear();
	type.FillOutlineMesh(outl_buf);
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
	//std::stringstream s;
	//s << std::setprecision(3) << "pitch: " << rad2deg(ctrl.Pitch())
	//	<< ", yaw: " << rad2deg(ctrl.Yaw());
	//orientation_text.Set(env.assets.small_ui_font, s.str());
}

void HUD::PostMessage(const char *msg) {
	messages.PushLine(msg);
	msg_timer.Reset();
	msg_timer.Start();
	std::cout << msg << std::endl;
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
		PlainColor &outline_prog = viewport.WorldOutlineProgram();
		outline_prog.SetM(outline_transform);
		outline.Draw();
	}

	// clear depth buffer so everything renders above the world
	viewport.ClearDepth();

	if (config.video.hud) {
		// "inventory"
		if (block_visible) {
			DirectionalLighting &world_prog = viewport.HUDProgram();
			world_prog.SetLightDirection({ 1.0f, 3.0f, 5.0f });
			// disable distance fog
			world_prog.SetFogDensity(0.0f);

			viewport.DisableBlending();
			world_prog.SetM(block_transform);
			block.Draw();
			block_label.Render(viewport);
		}

		// message box
		if (msg_timer.Running()) {
			messages.Render(viewport);
		}

		// crosshair
		PlainColor &outline_prog = viewport.HUDOutlineProgram();
		viewport.EnableInvertBlending();
		viewport.SetCursor(glm::vec3(0.0f), Gravity::CENTER);
		outline_prog.SetM(viewport.Cursor());
		crosshair.Draw();
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
, slot(0)
, num_slots(10) {

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
			config.audio.enabled = !config.audio.enabled;
			client_ctrl.SetAudio(config.audio.enabled);
			break;
		case Keymap::TOGGLE_VIDEO:
			config.video.world = !config.video.world;
			client_ctrl.SetVideo(config.video.world);
			break;
		case Keymap::TOGGLE_HUD:
			config.video.hud = !config.video.hud;
			client_ctrl.SetHUD(config.video.hud);
			break;
		case Keymap::TOGGLE_DEBUG:
			config.video.debug = !config.video.debug;
			client_ctrl.SetDebug(config.video.debug);
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
	if (!config.input.mouse) return;
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
	slot = s % num_slots;
	while (slot < 0) {
		slot += num_slots;
	}
	player_ctrl.SelectInventory(slot);
}

void Interface::InvRel(int delta) {
	InvAbs(slot + delta);
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
	Map(SDL_SCANCODE_RETURN, SECONDARY);
	Map(SDL_SCANCODE_MENU, TERTIARY);
	Map(SDL_SCANCODE_DELETE, PRIMARY);
	Map(SDL_SCANCODE_BACKSPACE, PRIMARY);

	Map(SDL_SCANCODE_F1, TOGGLE_HUD);
	Map(SDL_SCANCODE_F2, TOGGLE_VIDEO);
	Map(SDL_SCANCODE_F3, TOGGLE_DEBUG);
	Map(SDL_SCANCODE_F4, TOGGLE_AUDIO);

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
