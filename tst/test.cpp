#include <cstring>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

using CppUnit::TestFactoryRegistry;
using CppUnit::TextUi::TestRunner;


int main(int argc, char **argv) {

	bool headless = false;
	if (argc > 1 && std::strcmp(argv[1], "--headless") == 0) {
		headless = true;
	}

	TestRunner runner;
	{
		TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
		runner.addTest(registry.makeTest());
	}
	if (!headless) {
		TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry("headed");
		runner.addTest(registry.makeTest());
	}

	if (runner.run()) {
		return 0;
	} else {
		return 1;
	}
}
