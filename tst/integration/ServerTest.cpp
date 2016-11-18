#include "ServerTest.hpp"

#include "TestServer.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::ServerTest);


namespace blank {
namespace test {

void ServerTest::setUp() {

}

void ServerTest::tearDown() {

}


void ServerTest::testStartup() {
	TestServer server;
}

}
}
