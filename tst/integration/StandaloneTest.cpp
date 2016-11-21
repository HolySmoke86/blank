#include "StandaloneTest.hpp"

#include "TestInstance.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::StandaloneTest);


namespace blank {
namespace test {

void StandaloneTest::setUp() {

}

void StandaloneTest::tearDown() {

}


void StandaloneTest::testStartup() {
	TestInstance standalone({ });
	standalone.AssertRunning();
	standalone.AssertOutputLine("chunk preloading complete");
	standalone.Terminate();
	standalone.AssertExitStatus(0);
	// can't do that because AL blurts out nonsense
	//standalone.AssertNoError();
}

}
}
