#ifndef BLANK_TEST_INTEGRATION_TESTSERVER_HPP_
#define BLANK_TEST_INTEGRATION_TESTSERVER_HPP_

#include "app/Process.hpp"
#include "io/filesystem.hpp"
#include "io/LineBuffer.hpp"
#include "net/tcp.hpp"


namespace blank {
namespace test {

class TestServer {

public:
	TestServer();
	~TestServer();

public:
	/// wait until server writes given line to stdout
	void WaitOutputLine(const std::string &line);

	/// wait for given message on the command service
	void WaitCommandMessage(const std::string &line);
	/// wait for given error on the command service
	void WaitCommandError(const std::string &line);
	/// wait for given broadcast on the command service
	void WaitCommandBroadcast(const std::string &line);
	/// wait for given line on the command service
	void WaitCommandLine(const std::string &line);

	/// send command line to server
	void SendCommand(const std::string &);

private:
	TempDir dir;
	Process proc;
	tcp::Socket conn;
	size_t head;
	LineBuffer<BUFSIZ> serv_buf;
	LineBuffer<BUFSIZ> sock_buf;

};

}
}

#endif
