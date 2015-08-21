#include "HUD.hpp"
#include "Interface.hpp"
#include "Keymap.hpp"

#include "../app/Assets.hpp"
#include "../app/Environment.hpp"
#include "../app/FrameCounter.hpp"
#include "../app/init.hpp"
#include "../audio/Audio.hpp"
#include "../graphics/Font.hpp"
#include "../graphics/Viewport.hpp"
#include "../io/TokenStreamReader.hpp"
#include "../model/shapes.hpp"
#include "../world/BlockLookup.hpp"
#include "../world/World.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>


namespace blank {

HUD::HUD(const BlockTypeRegistry &types, const Font &font)
: types(types)
, font(font)
, block()
, block_buf()
, block_transform(1.0f)
, block_label()
, block_visible(false)
, crosshair() {
	block_transform = glm::translate(block_transform, glm::vec3(50.0f, 50.0f, 0.0f));
	block_transform = glm::scale(block_transform, glm::vec3(50.0f));
	block_transform = glm::rotate(block_transform, 3.5f, glm::vec3(1.0f, 0.0f, 0.0f));
	block_transform = glm::rotate(block_transform, 0.35f, glm::vec3(0.0f, 1.0f, 0.0f));

	OutlineModel::Buffer buf;
	buf.vertices = std::vector<glm::vec3>({
		{ -10.0f,   0.0f, 0.0f }, { 10.0f,  0.0f, 0.0f },
		{   0.0f, -10.0f, 0.0f }, {  0.0f, 10.0f, 0.0f },
	});
	buf.indices = std::vector<OutlineModel::Index>({
		0, 1, 2, 3
	});
	buf.colors.resize(4, { 10.0f, 10.0f, 10.0f });
	crosshair.Update(buf);

	block_label.Position(
		glm::vec3(50.0f, 85.0f, 0.0f),
		Gravity::NORTH_WEST,
		Gravity::NORTH
	);
	block_label.Foreground(glm::vec4(1.0f));
	block_label.Background(glm::vec4(0.5f));
}


void HUD::Display(const Block &b) {
	const BlockType &type = types.Get(b.type);

	block_buf.Clear();
	type.FillEntityModel(block_buf, b.Transform());
	block.Update(block_buf);

	block_label.Set(font, type.label);

	block_visible = type.visible;
}


void HUD::Render(Viewport &viewport) noexcept {
	viewport.ClearDepth();

	PlainColor &outline_prog = viewport.HUDOutlineProgram();
	viewport.EnableInvertBlending();
	viewport.SetCursor(glm::vec3(0.0f), Gravity::CENTER);
	outline_prog.SetM(viewport.Cursor());
	crosshair.Draw();

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
}


Interface::Interface(
	const Config &config,
	Environment &env,
	World &world)
: env(env)
, world(world)
, ctrl(world.Player())
, hud(world.BlockTypes(), env.assets.small_ui_font)
, aim{{ 0, 0, 0 }, { 0, 0, -1 }}
, aim_world()
, aim_entity()
, outline()
, outline_transform(1.0f)
, counter_text()
, position_text()
, orientation_text()
, block_text()
, last_block()
, last_entity(nullptr)
, messages(env.assets.small_ui_font)
, msg_timer(5000)
, config(config)
, place_timer(256)
, remove_timer(256)
, remove(0)
, selection(1)
, place_sound(env.assets.LoadSound("thump"))
, remove_sound(env.assets.LoadSound("plop"))
, fwd(0)
, rev(0)
, debug(false) {
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
	messages.Position(glm::vec3(25.0f, -25.0f, 0.0f), Gravity::SOUTH_WEST);
	messages.Foreground(glm::vec4(1.0f));
	messages.Background(glm::vec4(0.5f));
	hud.Display(selection);
}


void Interface::HandlePress(const SDL_KeyboardEvent &event) {
	if (config.keyboard_disabled) return;

	switch (env.keymap.Lookup(event)) {
		case Keymap::MOVE_FORWARD:
			rev.z = 1;
			break;
		case Keymap::MOVE_BACKWARD:
			fwd.z = 1;
			break;
		case Keymap::MOVE_LEFT:
			rev.x = 1;
			break;
		case Keymap::MOVE_RIGHT:
			fwd.x = 1;
			break;
		case Keymap::MOVE_UP:
			fwd.y = 1;
			break;
		case Keymap::MOVE_DOWN:
			rev.y = 1;
			break;

		case Keymap::BLOCK_FACE:
			FaceBlock();
			break;
		case Keymap::BLOCK_TURN:
			TurnBlock();
			break;
		case Keymap::BLOCK_NEXT:
			SelectNext();
			break;
		case Keymap::BLOCK_PREV:
			SelectPrevious();
			break;

		case Keymap::BLOCK_PLACE:
			PlaceBlock();
			break;
		case Keymap::BLOCK_PICK:
			PickBlock();
			break;
		case Keymap::BLOCK_REMOVE:
			RemoveBlock();
			break;

		case Keymap::TOGGLE_COLLISION:
			ToggleCollision();
			break;

		case Keymap::PRINT_BLOCK:
			PrintBlockInfo();
			break;
		case Keymap::PRINT_CHUNK:
			PrintChunkInfo();
			break;
		case Keymap::PRINT_LIGHT:
			PrintLightInfo();
			break;
		case Keymap::PRINT_SELECTION:
			PrintSelectionInfo();
			break;

		case Keymap::TOGGLE_VISUAL:
			ToggleVisual();
			break;
		case Keymap::TOGGLE_DEBUG:
			ToggleDebug();
			break;
		case Keymap::TOGGLE_AUDIO:
			ToggleAudio();
			break;

		case Keymap::EXIT:
			env.state.Pop();
			break;

		default:
			break;
	}
}

void Interface::HandleRelease(const SDL_KeyboardEvent &event) {
	if (config.keyboard_disabled) return;

	switch (env.keymap.Lookup(event)) {
		case Keymap::MOVE_FORWARD:
			rev.z = 0;
			break;
		case Keymap::MOVE_BACKWARD:
			fwd.z = 0;
			break;
		case Keymap::MOVE_LEFT:
			rev.x = 0;
			break;
		case Keymap::MOVE_RIGHT:
			fwd.x = 0;
			break;
		case Keymap::MOVE_UP:
			fwd.y = 0;
			break;
		case Keymap::MOVE_DOWN:
			rev.y = 0;
			break;

		default:
			break;
	}
}

void Interface::FaceBlock() {
	selection.SetFace(Block::Face((selection.GetFace() + 1) % Block::FACE_COUNT));
	hud.Display(selection);
}

void Interface::TurnBlock() {
	selection.SetTurn(Block::Turn((selection.GetTurn() + 1) % Block::TURN_COUNT));
	hud.Display(selection);
}

void Interface::ToggleCollision() {
	ctrl.Controlled().WorldCollidable(!ctrl.Controlled().WorldCollidable());
	if (ctrl.Controlled().WorldCollidable()) {
		PostMessage("collision on");
	} else {
		PostMessage("collision off");
	}
}

void Interface::PrintBlockInfo() {
	std::cout << std::endl;
	if (!aim_world) {
		PostMessage("not looking at any block");
		Ray aim = ctrl.Aim();
		std::stringstream s;
		s << "aim ray: " << aim.orig << ", " << aim.dir;
		PostMessage(s.str());
		return;
	}
	std::stringstream s;
	s << "looking at block " << aim_world.block
		<< " " << aim_world.BlockCoords()
		<< " of chunk " << aim_world.GetChunk().Position()
	;
	PostMessage(s.str());
	Print(aim_world.GetBlock());
}

void Interface::PrintChunkInfo() {
	std::cout << std::endl;
	if (!aim_world) {
		PostMessage("not looking at any block");
		return;
	}
	std::stringstream s;
	s << "looking at chunk " << aim_world.GetChunk().Position();
	PostMessage(s.str());

	PostMessage("  neighbors:");
	if (aim_world.GetChunk().HasNeighbor(Block::FACE_LEFT)) {
		s.str("");
		s << " left  " << aim_world.GetChunk().GetNeighbor(Block::FACE_LEFT).Position();
		PostMessage(s.str());
	}
	if (aim_world.GetChunk().HasNeighbor(Block::FACE_RIGHT)) {
		s.str("");
		s << " right " << aim_world.GetChunk().GetNeighbor(Block::FACE_RIGHT).Position();
		PostMessage(s.str());
	}
	if (aim_world.GetChunk().HasNeighbor(Block::FACE_UP)) {
		s.str("");
		s << " up    " << aim_world.GetChunk().GetNeighbor(Block::FACE_UP).Position();
		PostMessage(s.str());
	}
	if (aim_world.GetChunk().HasNeighbor(Block::FACE_DOWN)) {
		s.str("");
		s << " down  " << aim_world.GetChunk().GetNeighbor(Block::FACE_DOWN).Position();
		PostMessage(s.str());
	}
	if (aim_world.GetChunk().HasNeighbor(Block::FACE_FRONT)) {
		s.str("");
		s << " front " << aim_world.GetChunk().GetNeighbor(Block::FACE_FRONT).Position();
		PostMessage(s.str());
	}
	if (aim_world.GetChunk().HasNeighbor(Block::FACE_BACK)) {
		s.str("");
		s << " back  " << aim_world.GetChunk().GetNeighbor(Block::FACE_BACK).Position();
		PostMessage(s.str());
	}
	std::cout << std::endl;
}

void Interface::PrintLightInfo() {
	std::stringstream s;
	s
		<< "light level " << world.PlayerChunk().GetLight(world.Player().Position())
		<< " at position " << world.Player().Position()
	;
	PostMessage(s.str());
}

void Interface::PrintSelectionInfo() {
	std::cout << std::endl;
	Print(selection);
}

void Interface::Print(const Block &block) {
	std::stringstream s;
	s << "type: " << block.type
		<< ", face: " << block.GetFace()
		<< ", turn: " << block.GetTurn()
	;
	PostMessage(s.str());
}

void Interface::ToggleAudio() {
	config.audio_disabled = !config.audio_disabled;
	if (config.audio_disabled) {
		PostMessage("audio off");
	} else {
		PostMessage("audio on");
	}
}

void Interface::ToggleVisual() {
	config.visual_disabled = !config.visual_disabled;
	if (config.visual_disabled) {
		PostMessage("visual off");
	} else {
		PostMessage("visual on");
	}
}

void Interface::ToggleDebug() {
	debug = !debug;
	if (debug) {
		UpdateCounter();
		UpdatePosition();
		UpdateOrientation();
		UpdateBlockInfo();
		UpdateEntityInfo();
	}
}

void Interface::UpdateCounter() {
	std::stringstream s;
	s << std::setprecision(3) <<
		"avg: " << env.counter.Average().running << "ms, "
		"peak: " << env.counter.Peak().running << "ms";
	std::string text = s.str();
	counter_text.Set(env.assets.small_ui_font, text);
}

void Interface::UpdatePosition() {
	std::stringstream s;
	s << std::setprecision(3) << "pos: " << ctrl.Controlled().AbsolutePosition();
	position_text.Set(env.assets.small_ui_font, s.str());
}

void Interface::UpdateOrientation() {
	std::stringstream s;
	s << std::setprecision(3) << "pitch: " << rad2deg(ctrl.Pitch())
		<< ", yaw: " << rad2deg(ctrl.Yaw());
	orientation_text.Set(env.assets.small_ui_font, s.str());
}

void Interface::UpdateBlockInfo() {
	if (aim_world) {
		const Block &block = aim_world.GetBlock();
		if (last_block != block) {
			std::stringstream s;
			s << "Block: "
				<< aim_world.GetType().label
				<< ", face: " << block.GetFace()
				<< ", turn: " << block.GetTurn();
			block_text.Set(env.assets.small_ui_font, s.str());
			last_block = block;
		}
	} else {
		if (last_block != Block()) {
			std::stringstream s;
			s << "Block: none";
			block_text.Set(env.assets.small_ui_font, s.str());
			last_block = Block();
		}
	}
}

void Interface::UpdateEntityInfo() {
	if (aim_entity) {
		if (last_entity != aim_entity.entity) {
			std::stringstream s;
			s << "Entity: " << aim_entity.entity->Name();
			entity_text.Set(env.assets.small_ui_font, s.str());
			last_entity = aim_entity.entity;
		}
	}
}


void Interface::Handle(const SDL_MouseMotionEvent &event) {
	if (config.mouse_disabled) return;
	ctrl.RotateYaw(event.xrel * config.yaw_sensitivity);
	ctrl.RotatePitch(event.yrel * config.pitch_sensitivity);
}

void Interface::HandlePress(const SDL_MouseButtonEvent &event) {
	if (config.mouse_disabled) return;

	if (event.button == SDL_BUTTON_LEFT) {
		RemoveBlock();
		remove_timer.Start();
	} else if (event.button == SDL_BUTTON_MIDDLE) {
		PickBlock();
	} else if (event.button == SDL_BUTTON_RIGHT) {
		PlaceBlock();
		place_timer.Start();
	}
}

void Interface::HandleRelease(const SDL_MouseButtonEvent &event) {
	if (config.mouse_disabled) return;

	if (event.button == SDL_BUTTON_LEFT) {
		remove_timer.Stop();
	} else if (event.button == SDL_BUTTON_RIGHT) {
		place_timer.Stop();
	}
}

void Interface::PickBlock() {
	if (!aim_world) return;
	selection = aim_world.GetBlock();
	hud.Display(selection);
}

void Interface::PlaceBlock() {
	if (!aim_world) return;

	glm::vec3 next_pos = aim_world.BlockCoords() + aim_world.normal;
	BlockLookup next_block(&aim_world.GetChunk(), next_pos);
	if (next_block) {
	}
	next_block.SetBlock(selection);

	if (config.audio_disabled) return;
	const Entity &player = ctrl.Controlled();
	env.audio.Play(
		place_sound,
		aim_world.GetChunk().ToSceneCoords(player.ChunkCoords(), next_pos)
	);
}

void Interface::RemoveBlock() noexcept {
	if (!aim_world) return;
	aim_world.SetBlock(remove);

	if (config.audio_disabled) return;
	const Entity &player = ctrl.Controlled();
	env.audio.Play(
		remove_sound,
		aim_world.GetChunk().ToSceneCoords(player.ChunkCoords(), aim_world.BlockCoords())
	);
}


void Interface::Handle(const SDL_MouseWheelEvent &event) {
	if (config.mouse_disabled) return;

	if (event.y < 0) {
		SelectNext();
	} else if (event.y > 0) {
		SelectPrevious();
	}
}

void Interface::SelectNext() {
	++selection.type;
	if (size_t(selection.type) >= world.BlockTypes().Size()) {
		selection.type = 1;
	}
	hud.Display(selection);
}

void Interface::SelectPrevious() {
	--selection.type;
	if (selection.type <= 0) {
		selection.type = world.BlockTypes().Size() - 1;
	}
	hud.Display(selection);
}


void Interface::PostMessage(const char *msg) {
	messages.PushLine(msg);
	msg_timer.Reset();
	msg_timer.Start();
	std::cout << msg << std::endl;
}


void Interface::Update(int dt) {
	ctrl.Velocity(glm::vec3(fwd - rev) * config.move_velocity);
	ctrl.Update(dt);

	msg_timer.Update(dt);
	place_timer.Update(dt);
	remove_timer.Update(dt);

	aim = ctrl.Aim();
	CheckAim();

	if (msg_timer.HitOnce()) {
		msg_timer.Stop();
	}

	if (remove_timer.Hit()) {
		RemoveBlock();
		CheckAim();
	}

	if (place_timer.Hit()) {
		PlaceBlock();
		CheckAim();
	}

	if (debug) {
		if (env.counter.Changed()) {
			UpdateCounter();
		}
		UpdatePosition();
		UpdateOrientation();
	}
}

namespace {

OutlineModel::Buffer outl_buf;

}

void Interface::CheckAim() {
	if (!world.Intersection(aim, glm::mat4(1.0f), ctrl.Controlled().ChunkCoords(), aim_world)) {
		aim_world = WorldCollision();
	}
	if (!world.Intersection(aim, glm::mat4(1.0f), ctrl.Controlled(), aim_entity)) {
		aim_entity = EntityCollision();
	}
	if (aim_world && aim_entity) {
		// got both, pick the closest one
		if (aim_world.depth < aim_entity.depth) {
			UpdateOutline();
			aim_entity = EntityCollision();
		} else {
			aim_world = WorldCollision();
		}
	} else if (aim_world) {
		UpdateOutline();
	}
	if (debug) {
		UpdateBlockInfo();
		UpdateEntityInfo();
	}
}

void Interface::UpdateOutline() {
	outl_buf.Clear();
	aim_world.GetType().FillOutlineModel(outl_buf);
	outline.Update(outl_buf);
	outline_transform = aim_world.GetChunk().Transform(world.Player().ChunkCoords());
	outline_transform *= aim_world.BlockTransform();
	outline_transform *= glm::scale(glm::vec3(1.005f));
}


void Interface::Render(Viewport &viewport) noexcept {
	if (config.visual_disabled) return;

	if (aim_world) {
		PlainColor &outline_prog = viewport.WorldOutlineProgram();
		outline_prog.SetM(outline_transform);
		outline.Draw();
	}

	if (debug) {
		counter_text.Render(viewport);
		position_text.Render(viewport);
		orientation_text.Render(viewport);
		if (aim_world) {
			block_text.Render(viewport);
		} else if (aim_entity) {
			entity_text.Render(viewport);
		}
	}

	if (msg_timer.Running()) {
		messages.Render(viewport);
	}

	hud.Render(viewport);
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

Keymap::Action Keymap::Lookup(SDL_Scancode scancode) {
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

	Map(SDL_SCANCODE_Q, BLOCK_FACE);
	Map(SDL_SCANCODE_E, BLOCK_TURN);
	Map(SDL_SCANCODE_TAB, BLOCK_NEXT);
	Map(SDL_SCANCODE_RIGHTBRACKET, BLOCK_NEXT);
	Map(SDL_SCANCODE_LEFTBRACKET, BLOCK_PREV);

	Map(SDL_SCANCODE_INSERT, BLOCK_PLACE);
	Map(SDL_SCANCODE_RETURN, BLOCK_PLACE);
	Map(SDL_SCANCODE_MENU, BLOCK_PICK);
	Map(SDL_SCANCODE_DELETE, BLOCK_REMOVE);
	Map(SDL_SCANCODE_BACKSPACE, BLOCK_REMOVE);

	Map(SDL_SCANCODE_N, TOGGLE_COLLISION);
	Map(SDL_SCANCODE_F1, TOGGLE_VISUAL);
	Map(SDL_SCANCODE_F3, TOGGLE_DEBUG);
	Map(SDL_SCANCODE_F4, TOGGLE_AUDIO);

	Map(SDL_SCANCODE_B, PRINT_BLOCK);
	Map(SDL_SCANCODE_C, PRINT_CHUNK);
	Map(SDL_SCANCODE_L, PRINT_LIGHT);
	Map(SDL_SCANCODE_P, PRINT_SELECTION);

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


const char *Keymap::ActionToString(Action action) {
	switch (action) {
		default:
		case NONE:
			return "none";
		case MOVE_FORWARD:
			return "move_forward";
		case MOVE_BACKWARD:
			return "move_backward";
		case MOVE_LEFT:
			return "move_left";
		case MOVE_RIGHT:
			return "move_right";
		case MOVE_UP:
			return "move_up";
		case MOVE_DOWN:
			return "move_down";
		case BLOCK_FACE:
			return "block_face";
		case BLOCK_TURN:
			return "block_turn";
		case BLOCK_NEXT:
			return "block_next";
		case BLOCK_PREV:
			return "block_prev";
		case BLOCK_PLACE:
			return "block_place";
		case BLOCK_PICK:
			return "block_pick";
		case BLOCK_REMOVE:
			return "block_remove";
		case TOGGLE_COLLISION:
			return "toggle_collision";
		case TOGGLE_AUDIO:
			return "toggle_audio";
		case TOGGLE_VISUAL:
			return "toggle_visual";
		case TOGGLE_DEBUG:
			return "toggle_debug";
		case PRINT_BLOCK:
			return "print_block";
		case PRINT_CHUNK:
			return "print_chunk";
		case PRINT_LIGHT:
			return "print_light";
		case PRINT_SELECTION:
			return "print_selection";
		case EXIT:
			return "exit";
	}
}

Keymap::Action Keymap::StringToAction(const std::string &str) {
	if (str == "move_forward") {
		return MOVE_FORWARD;
	} else if (str == "move_backward") {
		return MOVE_BACKWARD;
	} else if (str == "move_left") {
		return MOVE_LEFT;
	} else if (str == "move_right") {
		return MOVE_RIGHT;
	} else if (str == "move_up") {
		return MOVE_UP;
	} else if (str == "move_down") {
		return MOVE_DOWN;
	} else if (str == "block_face") {
		return BLOCK_FACE;
	} else if (str == "block_turn") {
		return BLOCK_TURN;
	} else if (str == "block_next") {
		return BLOCK_NEXT;
	} else if (str == "block_prev") {
		return BLOCK_PREV;
	} else if (str == "block_place") {
		return BLOCK_PLACE;
	} else if (str == "block_pick") {
		return BLOCK_PICK;
	} else if (str == "block_remove") {
		return BLOCK_REMOVE;
	} else if (str == "toggle_collision") {
		return TOGGLE_COLLISION;
	} else if (str == "toggle_audio") {
		return TOGGLE_AUDIO;
	} else if (str == "toggle_visual") {
		return TOGGLE_VISUAL;
	} else if (str == "toggle_debug") {
		return TOGGLE_DEBUG;
	} else if (str == "print_block") {
		return PRINT_BLOCK;
	} else if (str == "print_chunk") {
		return PRINT_CHUNK;
	} else if (str == "print_light") {
		return PRINT_LIGHT;
	} else if (str == "print_selection") {
		return PRINT_SELECTION;
	} else if (str == "exit") {
		return EXIT;
	} else {
		return NONE;
	}
}

}
