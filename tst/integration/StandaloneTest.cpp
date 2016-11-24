#include "StandaloneTest.hpp"

#include "TestInstance.hpp"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(blank::test::StandaloneTest, "integration");


namespace blank {
namespace test {

void StandaloneTest::setUp() {

}

void StandaloneTest::tearDown() {

}


void StandaloneTest::testStartup() {
	TestInstance standalone({ "--no-vsync" });
	standalone.AssertRunning();
	try {
		standalone.AssertOutputLine("chunk preloading complete");
		standalone.Terminate();
	} catch (...) {
		try {
			standalone.Terminate();
		} catch (...) { }
		std::string output;
		standalone.ExhaustError(output);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"process stderr",
			std::string(""), output);
		CPPUNIT_FAIL("exception in runtime");
	}
	standalone.AssertExitStatus(0);
	standalone.AssertNoError();
}

}
}
