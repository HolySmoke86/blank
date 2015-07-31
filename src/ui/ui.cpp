#include "HUD.hpp"
#include "Interface.hpp"

#include "../app/Assets.hpp"
#include "../app/FrameCounter.hpp"
#include "../app/init.hpp"
#include "../graphics/Font.hpp"
#include "../graphics/Viewport.hpp"
#include "../model/shapes.hpp"
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

	crosshair.vertices = std::vector<glm::vec3>({
		{ -10.0f,   0.0f, 0.0f }, { 10.0f,  0.0f, 0.0f },
		{   0.0f, -10.0f, 0.0f }, {  0.0f, 10.0f, 0.0f },
	});
	crosshair.indices = std::vector<OutlineModel::Index>({
		0, 1, 2, 3
	});
	crosshair.colors.resize(4, { 10.0f, 10.0f, 10.0f });
	crosshair.Invalidate();

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
	type.FillModel(block_buf, b.Transform());
	block.Update(block_buf);

	block_label.Set(font, type.label);

	block_visible = type.visible;
}


void HUD::Render(Viewport &viewport) noexcept {
	viewport.ClearDepth();

	DirectionalLighting &world_prog = viewport.HUDProgram();
	world_prog.SetLightDirection({ 1.0f, 3.0f, 5.0f });
	// disable distance fog
	world_prog.SetFogDensity(0.0f);

	viewport.EnableInvertBlending();
	viewport.SetCursor(glm::vec3(0.0f), Gravity::CENTER);
	world_prog.SetM(viewport.Cursor());
	crosshair.Draw();

	if (block_visible) {
		viewport.DisableBlending();
		world_prog.SetM(block_transform);
		block.Draw();
		block_label.Render(viewport);
	}
}


Interface::Interface(
	const Config &config,
	const Assets &assets,
	const FrameCounter &counter,
	World &world)
: counter(counter)
, world(world)
, ctrl(world.Player())
, font(assets.LoadFont("DejaVuSans", 16))
, hud(world.BlockTypes(), font)
, aim{{ 0, 0, 0 }, { 0, 0, -1 }}
, aim_chunk(nullptr)
, aim_block(0)
, aim_normal()
, outline()
, outline_transform(1.0f)
, counter_text()
, messages(font)
, msg_timer(5000)
, config(config)
, place_timer(256)
, remove_timer(256)
, remove(0)
, selection(1)
, fwd(0)
, rev(0) {
	counter_text.Hide();
	counter_text.Position(glm::vec3(-25.0f, 25.0f, 0.0f), Gravity::NORTH_EAST);
	counter_text.Foreground(glm::vec4(1.0f));
	counter_text.Background(glm::vec4(0.5f));
	messages.Position(glm::vec3(25.0f, -25.0f, 0.0f), Gravity::SOUTH_WEST);
	messages.Foreground(glm::vec4(1.0f));
	messages.Background(glm::vec4(0.5f));
	hud.Display(selection);
}


void Interface::HandlePress(const SDL_KeyboardEvent &event) {
	if (config.keyboard_disabled) return;

	switch (event.keysym.sym) {
		case SDLK_w:
			rev.z = 1;
			break;
		case SDLK_s:
			fwd.z = 1;
			break;
		case SDLK_a:
			rev.x = 1;
			break;
		case SDLK_d:
			fwd.x = 1;
			break;
		case SDLK_SPACE:
			fwd.y = 1;
			break;
		case SDLK_LSHIFT:
			rev.y = 1;
			break;

		case SDLK_q:
			FaceBlock();
			break;
		case SDLK_e:
			TurnBlock();
			break;

		case SDLK_n:
			ToggleCollision();
			break;

		case SDLK_b:
			PrintBlockInfo();
			break;
		case SDLK_c:
			PrintChunkInfo();
			break;
		case SDLK_l:
			PrintLightInfo();
			break;
		case SDLK_p:
			PrintSelectionInfo();
			break;

		case SDLK_F3:
			ToggleCounter();
			break;
	}
}

void Interface::HandleRelease(const SDL_KeyboardEvent &event) {
	if (config.keyboard_disabled) return;

	switch (event.keysym.sym) {
		case SDLK_w:
			rev.z = 0;
			break;
		case SDLK_s:
			fwd.z = 0;
			break;
		case SDLK_a:
			rev.x = 0;
			break;
		case SDLK_d:
			fwd.x = 0;
			break;
		case SDLK_SPACE:
			fwd.y = 0;
			break;
		case SDLK_LSHIFT:
			rev.y = 0;
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
	if (!aim_chunk) {
		PostMessage("not looking at any block");
		Ray aim = ctrl.Aim();
		std::stringstream s;
		s << "aim ray: " << aim.orig << ", " << aim.dir;
		PostMessage(s.str());
		return;
	}
	std::stringstream s;
	s << "looking at block " << aim_block
		<< " " << Chunk::ToCoords(aim_block)
		<< " of chunk " << aim_chunk->Position()
	;
	PostMessage(s.str());
	Print(aim_chunk->BlockAt(aim_block));
}

void Interface::PrintChunkInfo() {
	std::cout << std::endl;
	if (!aim_chunk) {
		PostMessage("not looking at any block");
		return;
	}
	std::stringstream s;
	s << "looking at chunk " << aim_chunk->Position();
	PostMessage(s.str());

	PostMessage("  neighbors:");
	if (aim_chunk->HasNeighbor(Block::FACE_LEFT)) {
		s.str("");
		s << " left  " << aim_chunk->GetNeighbor(Block::FACE_LEFT).Position();
		PostMessage(s.str());
	}
	if (aim_chunk->HasNeighbor(Block::FACE_RIGHT)) {
		s.str("");
		s << " right " << aim_chunk->GetNeighbor(Block::FACE_RIGHT).Position();
		PostMessage(s.str());
	}
	if (aim_chunk->HasNeighbor(Block::FACE_UP)) {
		s.str("");
		s << " up    " << aim_chunk->GetNeighbor(Block::FACE_UP).Position();
		PostMessage(s.str());
	}
	if (aim_chunk->HasNeighbor(Block::FACE_DOWN)) {
		s.str("");
		s << " down  " << aim_chunk->GetNeighbor(Block::FACE_DOWN).Position();
		PostMessage(s.str());
	}
	if (aim_chunk->HasNeighbor(Block::FACE_FRONT)) {
		s.str("");
		s << " front " << aim_chunk->GetNeighbor(Block::FACE_FRONT).Position();
		PostMessage(s.str());
	}
	if (aim_chunk->HasNeighbor(Block::FACE_BACK)) {
		s.str("");
		s << " back  " << aim_chunk->GetNeighbor(Block::FACE_BACK).Position();
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

void Interface::ToggleCounter() {
	counter_text.Toggle();
	if (counter_text.Visible()) {
		UpdateCounter();
	}
}

void Interface::UpdateCounter() {
	std::stringstream s;
	s << std::setprecision(3) <<
		"avg: " << counter.Average().running << "ms, "
		"peak: " << counter.Peak().running << "ms";
	std::string text = s.str();
	counter_text.Set(font, text);
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
	if (!aim_chunk) return;
	selection = aim_chunk->BlockAt(aim_block);
	hud.Display(selection);
}

void Interface::PlaceBlock() {
	if (!aim_chunk) return;
	Chunk *mod_chunk = aim_chunk;
	glm::vec3 next_pos = Chunk::ToCoords(aim_block) + aim_normal;
	if (!Chunk::InBounds(next_pos)) {
		mod_chunk = &world.Next(*aim_chunk, aim_normal);
		next_pos -= aim_normal * glm::vec3(Chunk::Extent());
	}
	mod_chunk->SetBlock(next_pos, selection);
	mod_chunk->Invalidate();
}

void Interface::RemoveBlock() noexcept {
	if (!aim_chunk) return;
	aim_chunk->SetBlock(aim_block, remove);
	aim_chunk->Invalidate();
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

	if (counter_text.Visible() && counter.Changed()) {
		UpdateCounter();
	}
}

void Interface::CheckAim() {
	float dist;
	if (world.Intersection(aim, glm::mat4(1.0f), aim_chunk, aim_block, dist, aim_normal)) {
		outline.Clear();
		aim_chunk->Type(aim_chunk->BlockAt(aim_block)).FillOutlineModel(outline);
		outline_transform = glm::scale(glm::vec3(1.0002f));
		outline_transform *= aim_chunk->Transform(world.Player().ChunkCoords());
		outline_transform *= aim_chunk->ToTransform(Chunk::ToPos(aim_block), aim_block);
	} else {
		aim_chunk = nullptr;
	}
}


void Interface::Render(Viewport &viewport) noexcept {
	if (config.visual_disabled) return;

	if (aim_chunk) {
		DirectionalLighting &world_prog = viewport.EntityProgram();
		world_prog.SetM(outline_transform);
		outline.Draw();
	}

	if (counter_text.Visible()) {
		counter_text.Render(viewport);
	}

	if (msg_timer.Running()) {
		messages.Render(viewport);
	}

	hud.Render(viewport);
}

}
