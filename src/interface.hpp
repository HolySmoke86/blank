#ifndef BLANK_INTERFACE_HPP_
#define BLANK_INTERFACE_HPP_

#include "block.hpp"
#include "controller.hpp"
#include "hud.hpp"
#include "model.hpp"
#include "shader.hpp"

#include <SDL.h>


namespace blank {

class Chunk;
class World;

class Interface {

public:
	explicit Interface(World &);

	void Handle(const SDL_KeyboardEvent &);
	void Handle(const SDL_MouseMotionEvent &);
	void Handle(const SDL_MouseButtonEvent &);
	void Handle(const SDL_MouseWheelEvent &);
	void Handle(const SDL_WindowEvent &);

	void FaceBlock();
	void TurnBlock();

	void PickBlock();
	void PlaceBlock();
	void RemoveBlock();

	void SelectNext();
	void SelectPrevious();

	void Update(int dt);

	void Render(DirectionalLighting &);

private:
	World &world;
	FPSController ctrl;
	HUD hud;

	Chunk *aim_chunk;
	int aim_block;
	glm::vec3 aim_normal;

	OutlineModel outline;
	glm::mat4 outline_transform;

	float move_velocity;
	float pitch_sensitivity;
	float yaw_sensitivity;

	Block remove;
	Block selection;

	bool front, back, left, right, up, down;

};

}

#endif
