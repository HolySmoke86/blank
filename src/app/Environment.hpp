#ifndef BLANK_APP_ENVIRONMENT_HPP_
#define BLANK_APP_ENVIRONMENT_HPP_

#include "Assets.hpp"
#include "FrameCounter.hpp"
#include "StateControl.hpp"
#include "../audio/Audio.hpp"
#include "../graphics/Viewport.hpp"

#include <string>


namespace blank {

class Window;

struct Environment {

	Audio audio;
	Viewport viewport;
	Window &window;

	Assets assets;
	FrameCounter counter;

	StateControl state;


	explicit Environment(Window &win, const std::string &asset_path);

};

}

#endif
