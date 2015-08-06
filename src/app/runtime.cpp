#include "Application.hpp"
#include "Environment.hpp"
#include "PreloadState.hpp"
#include "Runtime.hpp"
#include "WorldState.hpp"

#include "init.hpp"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <SDL.h>

using namespace std;


namespace {

string get_asset_path() {
	char *base = SDL_GetBasePath();
	string assets(base);
	assets += "assets/";
	SDL_free(base);
	return assets;
}

}

namespace blank {

Environment::Environment(Window &win)
: audio()
, viewport()
, window(win)
, assets(get_asset_path())
, counter() {

}


Runtime::Runtime() noexcept
: name("blank")
, mode(NORMAL)
, n(0)
, t(0)
, config() {

}


void Runtime::ReadArgs(int argc, const char *const *argv) {
	if (argc <= 0) return;
	name = argv[0];

	bool options = true;
	bool error = false;

	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		if (!arg || arg[0] == '\0') {
			cerr << "warning: found empty argument at position " << i << endl;
			continue;
		}
		if (options && arg[0] == '-') {
			if (arg[1] == '\0') {
				cerr << "warning: incomplete option list at position " << i << endl;
			} else if (arg[1] == '-') {
				if (arg[2] == '\0') {
					// stopper
					options = false;
				} else {
					// long option
					if (strcmp(arg + 2, "no-vsync") == 0) {
						config.vsync = false;
					} else if (strcmp(arg + 2, "no-keyboard") == 0) {
						config.interface.keyboard_disabled = true;
					} else if (strcmp(arg + 2, "no-mouse") == 0) {
						config.interface.mouse_disabled = true;
					} else if (strcmp(arg + 2, "no-hud") == 0) {
						config.interface.visual_disabled = true;
					} else if (strcmp(arg + 2, "no-audio") == 0) {
						config.interface.audio_disabled = true;
					} else {
						cerr << "unknown option " << arg << endl;
						error = true;
					}
				}
			} else {
				// short options
				for (int j = 1; arg[j] != '\0'; ++j) {
					switch (arg[j]) {
						case 'd':
							config.doublebuf = false;
							break;
						case 'm':
							++i;
							if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
								cerr << "missing argument to -m" << endl;
								error = true;
							} else {
								config.multisampling = strtoul(argv[i], nullptr, 10);
							}
							break;
						case 'n':
							++i;
							if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
								cerr << "missing argument to -n" << endl;
								error = true;
							} else {
								n = strtoul(argv[i], nullptr, 10);
							}
							break;
						case 's':
							++i;
							if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
								cerr << "missing argument to -s" << endl;
								error = true;
							} else {
								config.world.gen.solid_seed = strtoul(argv[i], nullptr, 10);
								config.world.gen.type_seed = config.world.gen.solid_seed;
							}
							break;
						case 't':
							++i;
							if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
								cerr << "missing argument to -t" << endl;
								error = true;
							} else {
								t = strtoul(argv[i], nullptr, 10);
							}
							break;
						case '-':
							// stopper
							options = false;
							break;
						default:
							cerr << "unknown option " << arg[j] << endl;
							error = true;
							break;
					}
				}
			}
		} else {
			cerr << "unable to interpret argument "
				<< i << " (" << arg << ")" << endl;
			error = true;
		}
	}

	if (error) {
		mode = ERROR;
		return;
	}

	if (n > 0) {
		if (t > 0) {
			mode = FIXED_FRAME_LIMIT;
		} else {
			mode = FRAME_LIMIT;
		}
	} else if (t > 0) {
		mode = TIME_LIMIT;
	} else {
		mode = NORMAL;
	}
}

int Runtime::Execute() {
	if (mode == ERROR) {
		return 1;
	}

	Init init(config.doublebuf, config.multisampling);

	Environment env(init.window);
	env.viewport.VSync(config.vsync);

	Application app(env);

	WorldState world_state(env, config.interface, config.world);
	app.PushState(&world_state);

	PreloadState preloader(env, world_state.GetWorld().Loader());
	app.PushState(&preloader);

	switch (mode) {
		default:
		case NORMAL:
			app.Run();
			break;
		case FRAME_LIMIT:
			app.RunN(n);
			break;
		case TIME_LIMIT:
			app.RunT(t);
			break;
		case FIXED_FRAME_LIMIT:
			app.RunS(n, t);
			break;
	}

	return 0;
}

}
