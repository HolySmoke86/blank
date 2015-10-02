#ifndef BLANK_APP_ENVIRONMENT_HPP_
#define BLANK_APP_ENVIRONMENT_HPP_

#include "Assets.hpp"
#include "FrameCounter.hpp"
#include "MessageState.hpp"
#include "StateControl.hpp"
#include "../audio/Audio.hpp"
#include "../graphics/Viewport.hpp"
#include "../ui/Keymap.hpp"

#include <string>


namespace blank {

class Window;

struct HeadlessEnvironment {

	struct Config {
		std::string asset_path;
		std::string save_path;

		std::string GetWorldPath(
			const std::string &world_name
		) const;
		std::string GetWorldPath(
			const std::string &world_name,
			const std::string &hostname
		) const;
	} config;

	AssetLoader loader;

	FrameCounter counter;

	StateControl state;


	explicit HeadlessEnvironment(const Config &);

};


struct Environment
: public HeadlessEnvironment {

	Assets assets;

	Audio audio;
	Viewport viewport;
	Window &window;

	Keymap keymap;

	MessageState msg_state;


	Environment(Window &win, const Config &);

	void ShowMessage(const char *);

};

}

#endif
