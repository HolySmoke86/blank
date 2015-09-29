#ifndef BLANK_UI_INTERFACE_HPP_
#define BLANK_UI_INTERFACE_HPP_

#include "../app/Config.hpp"

#include <SDL.h>
#include <glm/glm.hpp>


namespace blank {

struct ClientController;
class Keymap;
struct PlayerController;

class Interface {

public:
	Interface(Config &, const Keymap &, PlayerController &, ClientController &);

	void SetInventorySlots(int num) { num_slots = num; }

	void HandlePress(const SDL_KeyboardEvent &);
	void HandleRelease(const SDL_KeyboardEvent &);
	void Handle(const SDL_MouseMotionEvent &);
	void HandlePress(const SDL_MouseButtonEvent &);
	void HandleRelease(const SDL_MouseButtonEvent &);
	void Handle(const SDL_MouseWheelEvent &);

private:
	void UpdateMovement();
	void InvAbs(int slot);
	void InvRel(int delta);

private:
	Config &config;
	const Keymap &keymap;
	PlayerController &player_ctrl;
	ClientController &client_ctrl;

	glm::ivec3 fwd, rev;
	int slot;
	int num_slots;

};

}

#endif
