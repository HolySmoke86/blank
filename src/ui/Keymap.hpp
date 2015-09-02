#ifndef BLANK_UI_KEYMAP_HPP_
#define BLANK_UI_KEYMAP_HPP_

#include <iosfwd>
#include <string>
#include <SDL.h>


namespace blank {

class Keymap {

public:
	enum Action {
		NONE,

		MOVE_FORWARD,
		MOVE_BACKWARD,
		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_UP,
		MOVE_DOWN,

		BLOCK_FACE,
		BLOCK_TURN,
		BLOCK_NEXT,
		BLOCK_PREV,

		BLOCK_PLACE,
		BLOCK_PICK,
		BLOCK_REMOVE,

		TOGGLE_COLLISION,
		TOGGLE_AUDIO,
		TOGGLE_VISUAL,
		TOGGLE_DEBUG,

		EXIT,
	};

	static constexpr unsigned int MAX_SCANCODE = 0xFF;
	static constexpr unsigned int NUM_SCANCODES = MAX_SCANCODE + 1;

public:
	Keymap();

	void Map(SDL_Scancode scancode, Action);
	Action Lookup(SDL_Scancode scancode);
	Action Lookup(const SDL_Keysym &s) { return Lookup(s.scancode); }
	Action Lookup(const SDL_KeyboardEvent &e) { return Lookup(e.keysym); }

	void LoadDefault();

	void Load(std::istream &);
	void Save(std::ostream &);

	static const char *ActionToString(Action);
	static Action StringToAction(const std::string &);

private:
	Action codemap[NUM_SCANCODES];

};

}

#endif
