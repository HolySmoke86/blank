#ifndef BLANK_UI_INTERFACE_HPP_
#define BLANK_UI_INTERFACE_HPP_

#include "HUD.hpp"
#include "../app/FPSController.hpp"
#include "../app/IntervalTimer.hpp"
#include "../model/geometry.hpp"
#include "../model/OutlineModel.hpp"
#include "../world/Block.hpp"

#include <SDL.h>
#include <glm/glm.hpp>


namespace blank {

class Chunk;
class DirectionalLighting;
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

	void HandlePress(const SDL_KeyboardEvent &);
	void HandleRelease(const SDL_KeyboardEvent &);
	void Handle(const SDL_MouseMotionEvent &);
	void HandlePress(const SDL_MouseButtonEvent &);
	void HandleRelease(const SDL_MouseButtonEvent &);
	void Handle(const SDL_MouseWheelEvent &);
	void Handle(const SDL_WindowEvent &) noexcept;

	void FaceBlock();
	void TurnBlock();

	void ToggleCollision();

	void PickBlock();
	void PlaceBlock();
	void RemoveBlock() noexcept;

	void PrintBlockInfo();
	void PrintChunkInfo();
	void PrintLightInfo();
	void PrintSelectionInfo();
	void Print(const Block &);

	void SelectNext();
	void SelectPrevious();

	void Update(int dt);

	void Render(DirectionalLighting &) noexcept;

private:
	void CheckAim();

private:
	World &world;
	FPSController ctrl;
	HUD hud;

	Ray aim;
	Chunk *aim_chunk;
	int aim_block;
	glm::vec3 aim_normal;

	OutlineModel outline;
	glm::mat4 outline_transform;

	Config config;

	IntervalTimer place_timer;
	IntervalTimer remove_timer;

	Block remove;
	Block selection;

	glm::tvec3<int> fwd, rev;

};

}

#endif
