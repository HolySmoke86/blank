#ifndef BLANK_RUNTIME_HPP_
#define BLANK_RUNTIME_HPP_

#include "Application.hpp"

#include <cstddef>


namespace blank {

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

	Runtime() noexcept;

	void ReadArgs(int argc, const char *const *argv);

	int Execute();

private:
	const char *name;
	Mode mode;
	std::size_t n;
	std::size_t t;
	Application::Config config;

};

}

#endif
