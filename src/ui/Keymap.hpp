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

		PRIMARY,
		SECONDARY,
		TERTIARY,

		INV_NEXT,
		INV_PREVIOUS,
		INV_1,
		INV_2,
		INV_3,
		INV_4,
		INV_5,
		INV_6,
		INV_7,
		INV_8,
		INV_9,
		INV_10,

		TOGGLE_AUDIO,
		TOGGLE_VIDEO,
		TOGGLE_HUD,
		TOGGLE_DEBUG,

		EXIT,
	};

	static constexpr unsigned int MAX_SCANCODE = 0xFF;
	static constexpr unsigned int NUM_SCANCODES = MAX_SCANCODE + 1;

public:
	Keymap();

	void Map(SDL_Scancode scancode, Action);
	Action Lookup(SDL_Scancode scancode) const;
	Action Lookup(const SDL_Keysym &s) const { return Lookup(s.scancode); }
	Action Lookup(const SDL_KeyboardEvent &e) const { return Lookup(e.keysym); }

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
