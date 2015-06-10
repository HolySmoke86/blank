#include "app/Runtime.hpp"

using namespace blank;

int main(int argc, char *argv[]) {
	Runtime rt;
	rt.ReadArgs(argc, argv);
	return rt.Execute();
}
