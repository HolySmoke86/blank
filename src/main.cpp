#include "app.hpp"

#include <cctype>
#include <cstdlib>
#include <iostream>

using namespace blank;


namespace {

enum Mode {
	NORMAL,
	FRAME_LIMIT,
	TIME_LIMIT,
	FIXED_FRAME_LIMIT,
};

}


int main(int argc, char *argv[]) {

	Mode mode = NORMAL;
	size_t n = 0, t = 0;

	bool error = false;
	for (int i = 1; i < argc; ++i) {
		if (argv[i] == nullptr || argv[i][0] == '\0') continue;
		if (argv[i][0] == '-') {
			if (argv[i][1] == 't' && argv[i][2] == '\0') {
				++i;
				if (i >= argc) {
					std::cerr << "missing argument to -t" << std::endl;
					error = true;
				} else {
					t = std::strtoul(argv[i], nullptr, 10);
				}
			} else {
				std::cerr << "unable to interpret argument "
					<< i << " (" << argv[i] << ")" << std::endl;
				error = true;
			}
		} else if (std::isdigit(*argv[i])) {
			n = std::strtoul(argv[i], nullptr, 10);
		} else {
			std::cerr << "unable to interpret argument "
				<< i << " (" << argv[i] << ")" << std::endl;
			error = true;
		}
	}

	if (error) {
		return 1;
	}

	if (n > 0) {
		if (t > 0) {
			mode = FIXED_FRAME_LIMIT;
		} else {
			mode = FRAME_LIMIT;
		}
	} else if (t > 0) {
		mode = TIME_LIMIT;
	}

	Application app;
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
