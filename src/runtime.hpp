#ifndef BLANK_RUNTIME_HPP_
#define BLANK_RUNTIME_HPP_

#include "app.hpp"

#include <cstddef>


namespace blank {

class Runtime {

public:
	enum Mode {
		NORMAL,
		FRAME_LIMIT,
		TIME_LIMIT,
		FIXED_FRAME_LIMIT,
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
