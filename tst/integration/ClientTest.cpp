#include "ClientTest.hpp"

#include "TestInstance.hpp"

#include <thread>

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(blank::test::ClientTest, "headed");


namespace blank {
namespace test {

void ClientTest::setUp() {
	server.reset(new TestInstance({ "--server" }, true));
	server->AssertRunning();
	server->AssertOutputLine("loading spawn chunks");
	server->AssertOutputLine("listening on UDP port 12354");
	client.reset(new TestInstance({ "--client" }));
	client->AssertRunning();
	client->AssertOutputLine("got message before interface was created: player \"default\" joined");
	client->AssertOutputLine("joined game \"default\"");
	server->AssertOutputLine("player \"default\" joined");
	server->AssertOutputLine("accepted login from player \"default\"");
}

void ClientTest::tearDown() {
	std::unique_ptr<TestInstance> srv(std::move(server));
	std::unique_ptr<TestInstance> cln(std::move(client));
	if (cln) {
		cln->Terminate();
		cln->AssertNoOutput();
		cln->AssertNoError();
		cln->AssertExitStatus(0);
	}
	if (srv) {
		srv->Terminate();
		srv->AssertOutputLine("player \"default\" left");
		srv->AssertOutputLine("saving remaining chunks");
		srv->AssertNoOutput();
		srv->AssertNoError();
		srv->AssertExitStatus(0);
	}
}


void ClientTest::testStartup() {

}

}
}
