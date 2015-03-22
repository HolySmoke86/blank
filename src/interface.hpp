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
	struct Config {
		float move_velocity = 0.005f;
		float pitch_sensitivity = -0.0025f;
		float yaw_sensitivity = -0.001f;

		bool keyboard_disabled = false;
		bool mouse_disabled = false;
		bool visual_disabled = false;
	};

	Interface(const Config &, World &);

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

	void PrintBlockInfo();
	void PrintChunkInfo();
	void PrintLightInfo();
	void PrintSelectionInfo();
	void Print(const Block &);

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

	Config config;

	Block remove;
	Block selection;

	bool front, back, left, right, up, down;

};

}

#endif
