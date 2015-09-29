#include "Application.hpp"
#include "Environment.hpp"
#include "Runtime.hpp"

#include "init.hpp"
#include "../client/MasterState.hpp"
#include "../io/filesystem.hpp"
#include "../io/WorldSave.hpp"
#include "../server/ServerState.hpp"
#include "../standalone/MasterState.hpp"

#include <cctype>
#include <cstdlib>
#include <fstream>
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

HeadlessEnvironment::HeadlessEnvironment(const Config &config)
: config(config)
, loader(config.asset_path)
, counter()
, state() {

}

string HeadlessEnvironment::Config::GetWorldPath(const string &world_name) const {
	return save_path + "worlds/" + world_name + '/';
}

string HeadlessEnvironment::Config::GetWorldPath(const string &world_name, const string &host_name) const {
	return save_path + "cache/" + host_name + '/' + world_name + '/';
}

Environment::Environment(Window &win, const Config &config)
: HeadlessEnvironment(config)
, assets(loader)
, audio()
, viewport()
, window(win)
, keymap() {
	viewport.Clear();
	window.Flip();
	keymap.LoadDefault();

	string keys_path = config.save_path + "keys.conf";
	if (!is_file(keys_path)) {
		std::ofstream file(keys_path);
		keymap.Save(file);
	} else {
		std::ifstream file(keys_path);
		keymap.Load(file);
	}
}


Runtime::Runtime() noexcept
: name("blank")
, mode(NORMAL)
, target(STANDALONE)
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
						config.game.video.vsync = false;
					} else if (strcmp(param, "no-keyboard") == 0) {
						config.game.input.keyboard = false;
					} else if (strcmp(param, "no-mouse") == 0) {
						config.game.input.mouse = false;
					} else if (strcmp(param, "no-hud") == 0) {
						config.game.video.hud = false;
					} else if (strcmp(param, "no-audio") == 0) {
						config.game.audio.enabled = false;
					} else if (strcmp(param, "standalone") == 0) {
						target = STANDALONE;
					} else if (strcmp(param, "server") == 0) {
						target = SERVER;
					} else if (strcmp(param, "client") == 0) {
						target = CLIENT;
					} else if (strcmp(param, "asset-path") == 0) {
						++i;
						if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
							cerr << "missing argument to --asset-path" << endl;
							error = true;
						} else {
							config.env.asset_path = argv[i];
						}
					} else if (strcmp(param, "host") == 0) {
						++i;
						if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
							cerr << "missing argument to --host" << endl;
							error = true;
						} else {
							config.game.net.host = argv[i];
						}
					} else if (strcmp(param, "port") == 0) {
						++i;
						if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
							cerr << "missing argument to --port" << endl;
							error = true;
						} else {
							config.game.net.port = strtoul(argv[i], nullptr, 10);
						}
					} else if (strcmp(param, "player-name") == 0) {
						++i;
						if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
							cerr << "missing argument to --player-name" << endl;
							error = true;
						} else {
							config.game.player.name = argv[i];
						}
					} else if (strcmp(param, "save-path") == 0) {
						++i;
						if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
							cerr << "missing argument to --save-path" << endl;
							error = true;
						} else {
							config.env.save_path = argv[i];
						}
					} else if (strcmp(param, "world-name") == 0) {
						++i;
						if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
							cerr << "missing argument to --world-name" << endl;
							error = true;
						} else {
							config.world.name = argv[i];
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
							config.game.video.dblbuf = false;
							break;
						case 'm':
							++i;
							if (i >= argc || argv[i] == nullptr || argv[i][0] == '\0') {
								cerr << "missing argument to -m" << endl;
								error = true;
							} else {
								config.game.video.msaa = strtoul(argv[i], nullptr, 10);
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
								config.gen.seed = strtoul(argv[i], nullptr, 10);
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

	if (config.env.asset_path.empty()) {
		config.env.asset_path = default_asset_path();
	} else if (
		config.env.asset_path[config.env.asset_path.size() - 1] != '/' &&
		config.env.asset_path[config.env.asset_path.size() - 1] != '\\'
	) {
		config.env.asset_path += '/';
	}
	if (config.env.save_path.empty()) {
		config.env.save_path = default_save_path();
	} else if (
		config.env.save_path[config.env.save_path.size() - 1] != '/' &&
		config.env.save_path[config.env.save_path.size() - 1] != '\\'
	) {
		config.env.save_path += '/';
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

	InitHeadless init_headless;

	switch (target) {
		default:
		case STANDALONE:
			RunStandalone();
			break;
		case SERVER:
			RunServer();
			break;
		case CLIENT:
			RunClient();
			break;
	}

	return 0;
}

void Runtime::RunStandalone() {
	Init init(config.game.video.dblbuf, config.game.video.msaa);

	Environment env(init.window, config.env);
	env.viewport.VSync(config.game.video.vsync);

	WorldSave save(config.env.GetWorldPath(config.world.name));
	if (save.Exists()) {
		save.Read(config.world);
		save.Read(config.gen);
	} else {
		save.Write(config.world);
		save.Write(config.gen);
	}

	Application app(env);
	standalone::MasterState world_state(env, config.game, config.gen, config.world, save);
	app.PushState(&world_state);
	Run(app);
}

void Runtime::RunServer() {
	HeadlessEnvironment env(config.env);

	WorldSave save(config.env.GetWorldPath(config.world.name));
	if (save.Exists()) {
		save.Read(config.world);
		save.Read(config.gen);
	} else {
		save.Write(config.world);
		save.Write(config.gen);
	}

	HeadlessApplication app(env);
	server::ServerState server_state(env, config.gen, config.world, save, config.game);
	app.PushState(&server_state);
	Run(app);
}

void Runtime::RunClient() {
	Init init(config.game.video.dblbuf, config.game.video.msaa);

	Environment env(init.window, config.env);
	env.viewport.VSync(config.game.video.vsync);

	Application app(env);
	client::MasterState client_state(env, config.game, config.world);
	app.PushState(&client_state);
	Run(app);
}

void Runtime::Run(HeadlessApplication &app) {
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
}

}
