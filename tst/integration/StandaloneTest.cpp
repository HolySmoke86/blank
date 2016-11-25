#include "StandaloneTest.hpp"

#include "TestInstance.hpp"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(blank::test::StandaloneTest, "headed");


namespace blank {
namespace test {

void StandaloneTest::setUp() {
	instance.reset(new TestInstance({ "--standalone", "--no-vsync" }));
	instance->AssertRunning();
}

void StandaloneTest::tearDown() {
	std::unique_ptr<TestInstance> inst(std::move(instance));
	if (inst) {
		inst->Terminate();
		inst->AssertNoOutput();
		inst->AssertNoError();
		inst->AssertExitStatus(0);
	}
}


void StandaloneTest::testStartup() {
	instance->AssertOutputLine("chunk preloading complete");
}

}
}
