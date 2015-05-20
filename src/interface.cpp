#include "interface.hpp"

#include "geometry.hpp"
#include "world.hpp"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>


namespace blank {

Interface::Interface(const Config &config, World &world)
: world(world)
, ctrl(world.Player())
, hud(world.BlockTypes())
, aim_chunk(nullptr)
, aim_block(0)
, aim_normal()
, outline()
, outline_transform(1.0f)
, config(config)
, remove(0)
, selection(1)
, fwd(0)
, rev(0) {
	hud.Viewport(960, 600);
	hud.Display(selection);
}


void Interface::Handle(const SDL_KeyboardEvent &event) {
	if (config.keyboard_disabled) return;

	switch (event.keysym.sym) {
		case SDLK_w:
			rev.z = event.state == SDL_PRESSED;
			break;
		case SDLK_s:
			fwd.z = event.state == SDL_PRESSED;
			break;
		case SDLK_a:
			rev.x = event.state == SDL_PRESSED;
			break;
		case SDLK_d:
			fwd.x = event.state == SDL_PRESSED;
			break;
		case SDLK_SPACE:
			fwd.y = event.state == SDL_PRESSED;
			break;
		case SDLK_LSHIFT:
			rev.y = event.state == SDL_PRESSED;
			break;

		case SDLK_q:
			if (event.state == SDL_PRESSED) {
				FaceBlock();
			}
			break;
		case SDLK_e:
			if (event.state == SDL_PRESSED) {
				TurnBlock();
			}
			break;

		case SDLK_b:
			if (event.state == SDL_PRESSED) {
				PrintBlockInfo();
			}
			break;
		case SDLK_c:
			if (event.state == SDL_PRESSED) {
				PrintChunkInfo();
			}
			break;
		case SDLK_l:
			if (event.state == SDL_PRESSED) {
				PrintLightInfo();
			}
			break;
		case SDLK_p:
			if (event.state == SDL_PRESSED) {
				PrintSelectionInfo();
			}
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

void Interface::Handle(const SDL_MouseButtonEvent &event) {
	if (config.mouse_disabled) return;

	if (event.state != SDL_PRESSED) return;

	if (event.button == 1) {
		RemoveBlock();
	} else if (event.button == 2) {
		PickBlock();
	} else if (event.button == 3) {
		PlaceBlock();
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

void Interface::RemoveBlock() {
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

void Interface::Handle(const SDL_WindowEvent &event) {
	if (event.event == SDL_WINDOWEVENT_RESIZED) {
		hud.Viewport(event.data1, event.data2);
	}
}


void Interface::Update(int dt) {
	ctrl.Velocity(glm::vec3(fwd - rev) * config.move_velocity);
	ctrl.Update(dt);

	Ray aim = ctrl.Aim();
	float dist;
	if (world.Intersection(aim, glm::mat4(1.0f), &aim_chunk, &aim_block, &dist, &aim_normal)) {
		outline.Clear();
		aim_chunk->Type(aim_chunk->BlockAt(aim_block)).FillOutlineModel(outline);
		outline_transform = glm::scale(glm::mat4(1.0f), glm::vec3(1.0002f));
		outline_transform = aim_chunk->Transform(world.Player().ChunkCoords());
		outline_transform *= aim_chunk->ToTransform(aim_block);
	} else {
		aim_chunk = nullptr;
	}

}


void Interface::Render(DirectionalLighting &program) {
	if (config.visual_disabled) return;

	if (aim_chunk) {
		program.SetM(outline_transform);
		outline.Draw();
	}

	hud.Render(program);
}

}
