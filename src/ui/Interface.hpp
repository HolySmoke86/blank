#ifndef BLANK_UI_INTERFACE_HPP_
#define BLANK_UI_INTERFACE_HPP_

#include "FixedText.hpp"
#include "HUD.hpp"
#include "MessageBox.hpp"
#include "../app/FPSController.hpp"
#include "../app/IntervalTimer.hpp"
#include "../audio/Sound.hpp"
#include "../graphics/Font.hpp"
#include "../model/geometry.hpp"
#include "../model/OutlineModel.hpp"
#include "../world/Block.hpp"

#include <string>
#include <glm/glm.hpp>


namespace blank {

class Chunk;
class Environment;
class Viewport;
class World;

class Interface {

public:
	struct Config {
		float move_velocity = 0.005f;
		float pitch_sensitivity = -0.0025f;
		float yaw_sensitivity = -0.001f;

		bool keyboard_disabled = false;
		bool mouse_disabled = false;
		bool audio_disabled = false;
		bool visual_disabled = false;
	};

	Interface(const Config &, Environment &, World &);

	void HandlePress(const SDL_KeyboardEvent &);
	void HandleRelease(const SDL_KeyboardEvent &);
	void Handle(const SDL_MouseMotionEvent &);
	void HandlePress(const SDL_MouseButtonEvent &);
	void HandleRelease(const SDL_MouseButtonEvent &);
	void Handle(const SDL_MouseWheelEvent &);

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

	void ToggleAudio();
	void ToggleVisual();

	void ToggleDebug();
	void UpdateCounter();
	void UpdatePosition();

	void PostMessage(const char *);
	void PostMessage(const std::string &msg) {
		PostMessage(msg.c_str());
	}

	void Update(int dt);

	void Render(Viewport &) noexcept;

private:
	void CheckAim();

private:
	Environment &env;
	World &world;
	FPSController ctrl;
	Font font;
	HUD hud;

	Ray aim;
	Chunk *aim_chunk;
	int aim_block;
	glm::vec3 aim_normal;

	OutlineModel outline;
	glm::mat4 outline_transform;

	FixedText counter_text;
	FixedText position_text;
	MessageBox messages;
	IntervalTimer msg_timer;

	Config config;

	IntervalTimer place_timer;
	IntervalTimer remove_timer;

	Block remove;
	Block selection;

	Sound place_sound;
	Sound remove_sound;

	glm::tvec3<int> fwd, rev;

};

}

#endif
