#ifndef BLANK_TEST_INTEGRATION_TESTINSTANCE_HPP_
#define BLANK_TEST_INTEGRATION_TESTINSTANCE_HPP_

#include "app/Process.hpp"
#include "io/filesystem.hpp"
#include "io/LineBuffer.hpp"
#include "net/tcp.hpp"


namespace blank {
namespace test {

class TestInstance {

public:
	/// Launch a blank instance with given arguments (program name
	/// will be prepended by the constructor).
	/// If connect is true, a command service connection will be
	/// established during construction.
	explicit TestInstance(const Process::Arguments &args, bool connect = false);
	~TestInstance();

public:
	/// Write given text to program's stdin.
	/// Data is not modified, so if you want to push a line, be
	/// sure to include a newline character.
	void WriteInput(const std::string &data);

	/// read next line from program's stdout
	void ReadOutputLine(std::string &line);
	/// assert that the next line the program writes to stdout will
	/// be the given one (without a trailing newline character)
	void AssertOutputLine(const std::string &line);
	/// wait until program writes given line to stdout
	void WaitOutputLine(const std::string &line);
	/// read from program's stdout until EOF
	void ExhaustOutput(std::string &output);
	/// assert that the program produces no more output
	void AssertNoOutput();

	/// read next line from program's stderr
	void ReadErrorLine(std::string &line);
	/// assert that the next line the program writes to stderr will
	/// be the given one (without a trailing newline character)
	void AssertErrorLine(const std::string &line);
	/// wait until program writes given line to stderr
	void WaitErrorLine(const std::string &line);
	/// read from program's stdout until EOF
	void ExhaustError(std::string &error);
	/// assert that the program produces no more output on stderr
	void AssertNoError();

	/// send termination signal
	void Terminate();
	/// assert that the program has not exited
	void AssertRunning();
	/// assert that the program has exited
	void AssertTerminated();
	/// make sure the process terminated with given status
	void AssertExitStatus(int expected);

	// the following methods are only valid when a command service is connected

	/// wait for given message on the command service
	void WaitCommandMessage(const std::string &line);
	/// wait for given error on the command service
	void WaitCommandError(const std::string &line);
	/// wait for given broadcast on the command service
	void WaitCommandBroadcast(const std::string &line);
	/// wait for given line on the command service
	void WaitCommandLine(const std::string &line);

	/// send command line to program
	void SendCommand(const std::string &);

private:
	TempDir dir;
	Process proc;
	tcp::Socket conn;
	size_t head;
	LineBuffer<BUFSIZ> out_buf;
	LineBuffer<BUFSIZ> err_buf;
	LineBuffer<BUFSIZ> cmd_buf;

};

}
}

#endif
