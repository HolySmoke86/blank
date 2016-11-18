#include "TestServer.hpp"

#include <iostream>


namespace blank {
namespace test {

TestServer::TestServer()
: dir()
, proc(
	"./blank" BLANK_SUFFIX,
	{ "blank", "--server", "--save-path", dir.Path(), "--cmd-port", "12354" },
	{ })
, conn()
, serv_buf()
, sock_buf() {
	// wait for command service startup
	// TODO: timeouts for reading from process
	WaitOutputLine("listening on TCP port 12354");
	// connect to command service
	conn = tcp::Socket("localhost", 12354);
}

TestServer::~TestServer() {
	proc.Terminate();
}


void TestServer::WaitOutputLine(const std::string &expected) {
	std::string line;
	while (true) {
		if (!serv_buf.Extract(line)) {
			// buffer exhausted, fetch more data
			serv_buf.Update(proc.ReadOut(serv_buf.WriteHead(), serv_buf.Remain()));
			continue;
		}
		if (line == expected) {
			return;
		} else {
			std::cerr << "ignoring line: " << line << std::endl;
		}
	}
}

void TestServer::WaitCommandMessage(const std::string &line) {
	WaitCommandLine(" > " + line);
}

void TestServer::WaitCommandError(const std::string &line) {
	WaitCommandLine(" ! " + line);
}

void TestServer::WaitCommandBroadcast(const std::string &line) {
	WaitCommandLine(" @ " + line);
}

void TestServer::WaitCommandLine(const std::string &expected) {
	std::string line;
	while (true) {
		if (!serv_buf.Extract(line)) {
			// buffer exhausted, fetch more data
			serv_buf.Update(conn.Recv(serv_buf.WriteHead(), serv_buf.Remain()));
			continue;
		}
		if (line == expected) {
			return;
		}
	}
}

}
}
