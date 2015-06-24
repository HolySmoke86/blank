#include "HUD.hpp"
#include "Interface.hpp"

#include "../app/init.hpp"
#include "../graphics/DirectionalLighting.hpp"
#include "../model/shapes.hpp"
#include "../world/World.hpp"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>


namespace blank {

HUD::HUD(const BlockTypeRegistry &types)
: types(types)
, block()
, block_buf()
, block_transform(1.0f)
, block_visible(false)
, crosshair()
, crosshair_transform(1.0f)
, near(100.0f)
, far(-100.0f)
, projection(glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, near, far))
, view(glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0))) {
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
}


void HUD::Viewport(float width, float height) noexcept {
	Viewport(0, 0, width, height);
}

void HUD::Viewport(float x, float y, float width, float height) noexcept {
	projection = glm::ortho(x, width, height, y, near, far);
	crosshair_transform = glm::translate(glm::mat4(1.0f), glm::vec3(width * 0.5f, height * 0.5f, 0.0f));
}


void HUD::Display(const Block &b) {
	const BlockType &type = types.Get(b.type);

	block_buf.Clear();
	type.FillModel(block_buf, b.Transform());
	block.Update(block_buf);
	block_visible = type.visible;
}


void HUD::Render(DirectionalLighting &program) noexcept {
	if (block_visible) {
		program.SetLightDirection({ 1.0f, 3.0f, 5.0f });
		// disable distance fog
		program.SetFogDensity(0.0f);
		GLContext::ClearDepthBuffer();
		program.SetMVP(block_transform, view, projection);
		block.Draw();
		program.SetM(crosshair_transform);
		crosshair.Draw();
	}
}


Interface::Interface(const Config &config, World &world)
: world(world)
, ctrl(world.Player())
, hud(world.BlockTypes())
, aim{{ 0, 0, 0 }, { 0, 0, -1 }}
, aim_chunk(nullptr)
, aim_block(0)
, aim_normal()
, outline()
, outline_transform(1.0f)
, config(config)
, place_timer(256)
, remove_timer(256)
, remove(0)
, selection(1)
, fwd(0)
, rev(0) {
	hud.Viewport(960, 600);
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

void Interface::PrintBlockInfo() {
	std::cout << std::endl;
	if (!aim_chunk) {
		std::cout << "not looking at any block" << std::endl;
		Ray aim = ctrl.Aim();
		std::cout << "aim ray: " << aim.orig << ", " << aim.dir << std::endl;
		return;
	}
	std::cout << "looking at block " << aim_block
		<< " " << Chunk::ToCoords(aim_block)
		<< " of chunk " << aim_chunk->Position()
		<< std::endl;
	Print(aim_chunk->BlockAt(aim_block));
}

void Interface::PrintChunkInfo() {
	std::cout << std::endl;
	if (!aim_chunk) {
		std::cout << "not looking at any block" << std::endl;
		return;
	}
	std::cout << "looking at chunk "
		<< aim_chunk->Position()
		<< std::endl;

	std::cout << "  neighbors:" << std::endl;
	if (aim_chunk->HasNeighbor(Block::FACE_LEFT)) {
		std::cout << " left  " << aim_chunk->GetNeighbor(Block::FACE_LEFT).Position() << std::endl;
	}
	if (aim_chunk->HasNeighbor(Block::FACE_RIGHT)) {
		std::cout << " right " << aim_chunk->GetNeighbor(Block::FACE_RIGHT).Position() << std::endl;
	}
	if (aim_chunk->HasNeighbor(Block::FACE_UP)) {
		std::cout << " up    " << aim_chunk->GetNeighbor(Block::FACE_UP).Position() << std::endl;
	}
	if (aim_chunk->HasNeighbor(Block::FACE_DOWN)) {
		std::cout << " down  " << aim_chunk->GetNeighbor(Block::FACE_DOWN).Position() << std::endl;
	}
	if (aim_chunk->HasNeighbor(Block::FACE_FRONT)) {
		std::cout << " front " << aim_chunk->GetNeighbor(Block::FACE_FRONT).Position() << std::endl;
	}
	if (aim_chunk->HasNeighbor(Block::FACE_BACK)) {
		std::cout << " back  " << aim_chunk->GetNeighbor(Block::FACE_BACK).Position() << std::endl;
	}
	std::cout << std::endl;
}

void Interface::PrintLightInfo() {
	std::cout
		<< "light level " << world.PlayerChunk().GetLight(world.Player().Position())
		<< " at position " << world.Player().Position()
		<< std::endl;
}

void Interface::PrintSelectionInfo() {
	std::cout << std::endl;
	Print(selection);
}

void Interface::Print(const Block &block) {
	std::cout << "type: " << block.type
		<< ", face: " << block.GetFace()
		<< ", turn: " << block.GetTurn()
		<< std::endl;
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

void Interface::Handle(const SDL_WindowEvent &event) noexcept {
	if (event.event == SDL_WINDOWEVENT_RESIZED) {
		hud.Viewport(event.data1, event.data2);
	}
}


void Interface::Update(int dt) {
	ctrl.Velocity(glm::vec3(fwd - rev) * config.move_velocity);
	ctrl.Update(dt);

	place_timer.Update(dt);
	remove_timer.Update(dt);

	aim = ctrl.Aim();
	CheckAim();

	if (remove_timer.Hit()) {
		RemoveBlock();
		CheckAim();
	}

	if (place_timer.Hit()) {
		PlaceBlock();
		CheckAim();
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


void Interface::Render(DirectionalLighting &program) noexcept {
	if (config.visual_disabled) return;

	if (aim_chunk) {
		program.SetM(outline_transform);
		outline.Draw();
	}

	hud.Render(program);
}

}
