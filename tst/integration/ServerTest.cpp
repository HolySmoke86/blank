#include "ServerTest.hpp"

#include "TestInstance.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::ServerTest);


namespace blank {
namespace test {

void ServerTest::setUp() {
	instance.reset(new TestInstance({ "--server" }, true));
	instance->AssertRunning();
	instance->AssertOutputLine("loading spawn chunks");
	instance->AssertOutputLine("listening on UDP port 12354");
}

void ServerTest::tearDown() {
	std::unique_ptr<TestInstance> inst(std::move(instance));
	if (inst) {
		inst->Terminate();
		inst->AssertOutputLine("saving remaining chunks");
		inst->AssertNoOutput();
		inst->AssertNoError();
		inst->AssertExitStatus(0);
	}
}


void ServerTest::testStartup() {
	// setUp and testDown do all the tests
}

}
}
