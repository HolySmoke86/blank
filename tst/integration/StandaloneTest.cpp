#include "StandaloneTest.hpp"

#include "TestInstance.hpp"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(blank::test::StandaloneTest, "integration");


namespace blank {
namespace test {

void StandaloneTest::setUp() {
	instance.reset(new TestInstance({ "--no-vsync" }));
	instance->AssertRunning();
}

void StandaloneTest::tearDown() {
	std::unique_ptr<TestInstance> inst(std::move(instance));
	inst->Terminate();
	inst->AssertExitStatus(0);
	inst->AssertNoError();
}


void StandaloneTest::testStartup() {
	instance->AssertOutputLine("chunk preloading complete");
}

}
}
