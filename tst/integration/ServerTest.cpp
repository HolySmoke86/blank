#include "ServerTest.hpp"

#include "TestInstance.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::ServerTest);


namespace blank {
namespace test {

void ServerTest::setUp() {
	instance.reset(new TestInstance({ "--server" }, true));
	instance->AssertRunning();
}

void ServerTest::tearDown() {
	std::unique_ptr<TestInstance> inst(std::move(instance));
	inst->Terminate();
	inst->AssertExitStatus(0);
	inst->AssertNoError();
}


void ServerTest::testStartup() {
	// setUp and testDown do all the tests
}

}
}
