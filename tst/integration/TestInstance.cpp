#include "TestInstance.hpp"

#include <cppunit/extensions/HelperMacros.h>

using namespace std;


namespace blank {
namespace test {

namespace {

Process::Arguments combine_args(const TempDir &dir, const Process::Arguments &in, bool cmd) {
	Process::Arguments out;
	out.reserve(in.size() + (cmd ? 5 : 3));
	out.emplace_back("blank");
	out.insert(out.end(), in.begin(), in.end());
	out.emplace_back("--save-path");
	out.emplace_back(dir.Path());
	if (cmd) {
		out.emplace_back("--cmd-port");
		out.emplace_back("12354");
	}
	return out;
}

}

TestInstance::TestInstance(const Process::Arguments &args, bool cmd)
: dir()
, proc(
	"./blank" BLANK_SUFFIX,
	combine_args(dir, args, cmd),
	{ })
, conn()
, out_buf()
, err_buf()
, cmd_buf() {
	if (cmd) {
		// wait for command service startup
		// TODO: timeouts for reading from process
		WaitOutputLine("listening on TCP port 12354");
		// connect to command service
		conn = tcp::Socket("localhost", 12354);
	}
}

TestInstance::~TestInstance() {
	proc.Terminate();
}


void TestInstance::WriteInput(const string &data) {
	const char *i = data.c_str();
	const char *end = i + data.length();
	while (i != end) {
		size_t len = proc.WriteIn(i, end - i);
		i += len;
	}
}

void TestInstance::ReadOutputLine(string &line) {
	while (!out_buf.Extract(line)) {
		// buffer exhausted, fetch more data
		out_buf.Update(proc.ReadOut(out_buf.WriteHead(), out_buf.Remain()));
	}
}

void TestInstance::AssertOutputLine(const string &expected) {
	string line;
	ReadOutputLine(line);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected line in stdout",
		expected, line);
}

void TestInstance::WaitOutputLine(const string &expected) {
	string line;
	while (true) {
		ReadOutputLine(line);
		if (line == expected) {
			return;
		}
	}
}

void TestInstance::ReadErrorLine(string &line) {
	while (!err_buf.Extract(line)) {
		// buffer exhausted, fetch more data
		err_buf.Update(proc.ReadErr(err_buf.WriteHead(), err_buf.Remain()));
	}
}

void TestInstance::AssertErrorLine(const string &expected) {
	string line;
	ReadErrorLine(line);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected line in stderr",
		expected, line);
}

void TestInstance::WaitErrorLine(const string &expected) {
	string line;
	while (true) {
		ReadErrorLine(line);
		if (line == expected) {
			return;
		}
	}
}


void TestInstance::AssertExitStatus(int expected) {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected line in stderr",
		expected, proc.Join());
}


void TestInstance::WaitCommandMessage(const string &line) {
	WaitCommandLine(" > " + line);
}

void TestInstance::WaitCommandError(const string &line) {
	WaitCommandLine(" ! " + line);
}

void TestInstance::WaitCommandBroadcast(const string &line) {
	WaitCommandLine(" @ " + line);
}

void TestInstance::WaitCommandLine(const string &expected) {
	string line;
	while (true) {
		if (!cmd_buf.Extract(line)) {
			// buffer exhausted, fetch more data
			cmd_buf.Update(conn.Recv(cmd_buf.WriteHead(), cmd_buf.Remain()));
			continue;
		}
		if (line == expected) {
			return;
		}
	}
}

}
}
