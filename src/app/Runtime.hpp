#ifndef BLANK_RUNTIME_HPP_
#define BLANK_RUNTIME_HPP_

#include "Environment.hpp"
#include "../client/Client.hpp"
#include "../server/Server.hpp"
#include "../ui/Interface.hpp"
#include "../world/Generator.hpp"
#include "../world/World.hpp"

#include <cstddef>
#include <string>


namespace blank {

class HeadlessApplication;

/// Parse and interpret arguemnts, then set up the environment and execute.
class Runtime {

public:
	enum Mode {
		/// default behaviour: run until user quits, dynamic timesteps
		NORMAL,
		/// quit after n frames
		FRAME_LIMIT,
		/// quit after n milliseconds
		TIME_LIMIT,
		/// quit after n frames, use fixed timestap
		FIXED_FRAME_LIMIT,
		/// display error message and quit with failure
		ERROR,
	};

	enum Target {
		STANDALONE,
		SERVER,
		CLIENT,
	};

	struct Config {
		blank::Config game = blank::Config();
		HeadlessEnvironment::Config env = HeadlessEnvironment::Config();
		Generator::Config gen = Generator::Config();
		World::Config world = World::Config();
	};

	Runtime() noexcept;

	void Initialize(int argc, const char *const *argv);

	int Execute();

private:
	void ReadArgs(int argc, const char *const *argv);
	void ReadPreferences();

	void RunStandalone();
	void RunServer();
	void RunClient();

	void Run(HeadlessApplication &);

private:
	const char *name;
	Mode mode;
	Target target;
	std::size_t n;
	std::size_t t;
	Config config;

};

}

#endif
