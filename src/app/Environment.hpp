#ifndef BLANK_APP_ENVIRONMENT_HPP_
#define BLANK_APP_ENVIRONMENT_HPP_

#include "Assets.hpp"
#include "FrameCounter.hpp"
#include "StateControl.hpp"
#include "../audio/Audio.hpp"
#include "../graphics/Viewport.hpp"
#include "../ui/Keymap.hpp"

#include <string>


namespace blank {

class Window;

struct HeadlessEnvironment {

	AssetLoader loader;

	FrameCounter counter;

	StateControl state;


	explicit HeadlessEnvironment(const std::string &asset_path);

};


struct Environment
: public HeadlessEnvironment {

	Assets assets;

	Audio audio;
	Viewport viewport;
	Window &window;

	Keymap keymap;


	Environment(Window &win, const std::string &asset_path);

};

}

#endif
