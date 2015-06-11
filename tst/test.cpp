#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

using CppUnit::TestFactoryRegistry;
using CppUnit::TextUi::TestRunner;


int main(int, char **) {

	TestRunner runner;
	TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	runner.run();

	return 0;

}
