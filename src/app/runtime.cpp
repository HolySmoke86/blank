#include "Application.hpp"
#include "Environment.hpp"
#include "Runtime.hpp"
#include "WorldState.hpp"

#include "init.hpp"
#include "../io/WorldSave.hpp"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <SDL.h>

using namespace std;


namespace {

string default_asset_path() {
	char *base = SDL_GetBasePath();
	string assets(base);
	assets += "assets/";
	SDL_free(base);
	return assets;
}

string default_save_path() {
#ifndef NDEBUG
	char *base = SDL_GetBasePath();
	string save(base);
	save += "saves/";
	SDL_free(base);
	return save;
#else
	char *pref = SDL_GetPrefPath("localhorst", "blank");
	string save(pref);
	SDL_free(pref);
	return save;
#endif
}

}

namespace blank {

Environment::Environment(Window &win, const string &asset_path)
: audio()
, viewport()
, window(win)
, assets(asset_path)
, counter() {
	viewport.Clear();
	window.Flip();
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
					const char *param = arg + 2;
					// long option
					if (strcmp(param, "no-vsync") == 0) {
						config.vsync = false;
					} else if (strcmp(param, "no-keyboard") == 0) {
						config.interface.keyboard_disabled = true;
					} else if (strcmp(param, "no-mouse") == 0) {
						config.interface.mouse_disabled = true;
					} else if (strcmp(param, "no-hud") == 0) {
						config.interface.visual_disabled = true;
					} else if (strcmp(param, "no-audio") == 0) {
						config.interface.audio_disabled = true;
					} else if (strcmp(param, "asset-path") == 0) {
						++i;
						if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
							cerr << "missing argument to --asset-path" << endl;
							error = true;
						} else {
							config.asset_path = argv[i];
						}
					} else if (strcmp(param, "save-path") == 0) {
						++i;
						if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
							cerr << "missing argument to --save-path" << endl;
							error = true;
						} else {
							config.save_path = argv[i];
						}
					} else if (strcmp(param, "world-name") == 0) {
						++i;
						if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
							cerr << "missing argument to --world-name" << endl;
							error = true;
						} else {
							config.world_name = argv[i];
						}
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
								config.world.gen.seed = strtoul(argv[i], nullptr, 10);
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

	if (config.asset_path.empty()) {
		config.asset_path = default_asset_path();
	} else if (
		config.asset_path[config.asset_path.size() - 1] != '/' &&
		config.asset_path[config.asset_path.size() - 1] != '\\'
	) {
		config.asset_path += '/';
	}
	if (config.save_path.empty()) {
		config.save_path = default_save_path();
	} else if (
		config.save_path[config.save_path.size() - 1] != '/' &&
		config.save_path[config.save_path.size() - 1] != '\\'
	) {
		config.save_path += '/';
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

	Environment env(init.window, config.asset_path);
	env.viewport.VSync(config.vsync);

	WorldSave save(config.save_path + config.world_name + '/');
	if (save.Exists()) {
		save.Read(config.world);
	} else {
		save.Write(config.world);
	}

	Application app(env);

	WorldState world_state(env, config.interface, config.world, save);
	app.PushState(&world_state);

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
