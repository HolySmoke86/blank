#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

using CppUnit::TestFactoryRegistry;
using CppUnit::TextUi::TestRunner;


int main(int, char **) {
	TestRunner runner;
	TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	if (runner.run()) {
		return 0;
	} else {
		return 1;
	}
}
