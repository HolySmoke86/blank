#include "app/Runtime.hpp"

#include <exception>
#include <iostream>

using namespace blank;

int main(int argc, char *argv[]) {
	Runtime rt;
	try {
		rt.Initialize(argc, argv);
	} catch (std::exception &e) {
		std::cerr << "error in initialization: " << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "unknown error in initialization" << std::endl;
		return 1;
	}
	try {
		return rt.Execute();
	} catch (std::exception &e) {
		std::cerr << "error in execution: " << e.what() << std::endl;
		return 2;
	} catch (...) {
		std::cerr << "unknown error in execution" << std::endl;
		return 2;
	}
}
