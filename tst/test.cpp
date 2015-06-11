#include "app/init.hpp"

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

using CppUnit::TestFactoryRegistry;
using CppUnit::TextUi::TestRunner;


int main(int, char **) {
	blank::InitSDL sdl;
	blank::InitGL gl;
	blank::Window win;
	blank::GLContext ctx(win.CreateContext());
	blank::InitGLEW glew;

	TestRunner runner;
	TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	runner.run();

	return 0;

}
