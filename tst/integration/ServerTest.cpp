#include "ServerTest.hpp"

#include "TestInstance.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::ServerTest);


namespace blank {
namespace test {

void ServerTest::setUp() {

}

void ServerTest::tearDown() {

}


void ServerTest::testStartup() {
	TestInstance server({ "--server" }, true);
	server.AssertRunning();
	server.Terminate();
	server.AssertExitStatus(0);
	server.AssertNoError();
}

}
}
