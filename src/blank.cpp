#include "app/Runtime.hpp"

using namespace blank;

int main(int argc, char *argv[]) {
	Runtime rt;
	rt.Initialize(argc, argv);
	return rt.Execute();
}
