#include "InvocationTest.hpp"

#include "TestInstance.hpp"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(blank::test::InvocationTest, "integration");


namespace blank {
namespace test {

void InvocationTest::setUp() {

}

void InvocationTest::tearDown() {

}


void InvocationTest::testUnknownArg() {
	TestInstance prog({ "--worscht" });
	prog.AssertErrorLine("unknown option --worscht");
	prog.AssertExitStatus(1);
}

}
}
