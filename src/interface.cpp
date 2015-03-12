#include "interface.hpp"

#include "geometry.hpp"
#include "world.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace blank {

Interface::Interface(World &world)
: world(world)
, ctrl(world.Player())
, hud(world.BlockTypes())
, aim_chunk(nullptr)
, aim_block(0)
, aim_normal()
, outline()
, outline_transform(1.0f)
, move_velocity(0.005f)
, pitch_sensitivity(-0.0025f)
, yaw_sensitivity(-0.001f)
, remove(0)
, selection(1)
, front(false)
, back(false)
, left(false)
, right(false)
, up(false)
, down(false) {
	hud.Viewport(960, 600);
	hud.Display(selection);
}


void Interface::Handle(const SDL_KeyboardEvent &event) {
	switch (event.keysym.sym) {
		case SDLK_w:
			front = event.state == SDL_PRESSED;
			break;
		case SDLK_s:
			back = event.state == SDL_PRESSED;
			break;
		case SDLK_a:
			left = event.state == SDL_PRESSED;
			break;
		case SDLK_d:
			right = event.state == SDL_PRESSED;
			break;
		case SDLK_SPACE:
			up = event.state == SDL_PRESSED;
			break;
		case SDLK_LSHIFT:
			down = event.state == SDL_PRESSED;
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


void Interface::Handle(const SDL_MouseMotionEvent &event) {
	ctrl.RotateYaw(event.xrel * yaw_sensitivity);
	ctrl.RotatePitch(event.yrel * pitch_sensitivity);
}

void Interface::Handle(const SDL_MouseButtonEvent &event) {
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
	mod_chunk->BlockAt(next_pos) = selection;
	mod_chunk->Invalidate();
}

void Interface::RemoveBlock() {
	if (!aim_chunk) return;
	aim_chunk->BlockAt(aim_block) = remove;
	aim_chunk->Invalidate();
}


void Interface::Handle(const SDL_MouseWheelEvent &event) {
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
	glm::vec3 vel;
	if (right && !left) {
		vel.x = move_velocity;
	} else if (left && !right) {
		vel.x = -move_velocity;
	}
	if (up && !down) {
		vel.y = move_velocity;
	} else if (down && !up) {
		vel.y = -move_velocity;
	}
	if (back && !front) {
		vel.z = move_velocity;
	} else if (front && !back) {
		vel.z = -move_velocity;
	}
	ctrl.Velocity(vel);
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
	if (aim_chunk) {
		program.SetM(outline_transform);
		outline.Draw();
	}

	hud.Render(program);
}

}
