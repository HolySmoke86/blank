#include "TestInstance.hpp"

#include <cppunit/extensions/HelperMacros.h>

using namespace std;


namespace blank {
namespace test {

namespace {

Process::Arguments combine_args(
	const TempDir &dir,
	const Process::Arguments &in,
	bool cmd,
	bool temp_save
) {
	Process::Arguments out;
	out.reserve(in.size() + (cmd ? 5 : 3));
	out.emplace_back("blank");
	out.insert(out.end(), in.begin(), in.end());
	if (temp_save) {
		out.emplace_back("--save-path");
		out.emplace_back(dir.Path());
	}
	if (cmd) {
		out.emplace_back("--cmd-port");
		out.emplace_back("12354");
	}
	return out;
}

}

TestInstance::TestInstance(const Process::Arguments &args, bool cmd, bool temp_save)
: dir()
, proc("./blank" BLANK_SUFFIX, combine_args(dir, args, cmd, temp_save))
, conn()
, out_buf()
, err_buf()
, cmd_buf()
, name("blank" BLANK_SUFFIX) {
	if (cmd) {
		// wait for command service startup
		WaitOutputLine("listening on TCP port 12354");
		// connect to command service
		conn = tcp::Socket("localhost", 12354);
	}
	for (const auto &arg : args) {
		name += ' ';
		name += arg;
	}
}

TestInstance::~TestInstance() {
	proc.Terminate();
}


void TestInstance::WriteInput(const string &data) {
	AssertRunning();
	const char *i = data.c_str();
	const char *end = i + data.length();
	while (i != end) {
		size_t len = proc.WriteIn(i, end - i);
		if (len == 0) {
			throw runtime_error("failed write to stdin of " + name);
		}
		i += len;
	}
}

void TestInstance::ReadOutputLine(string &line) {
	while (!out_buf.Extract(line)) {
		// buffer exhausted, fetch more data
		int len = proc.ReadOut(out_buf.WriteHead(), out_buf.Remain(), 10000);
		if (len == 0) {
			throw runtime_error("failed read from stdout of " + name);
		}
		out_buf.Update(len);
	}
}

void TestInstance::AssertOutputLine(const string &expected) {
	string line;
	if (past_out.empty()) {
		ReadOutputLine(line);
	} else {
		line = past_out.front();
		past_out.pop_front();
	}
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected line in stdout of " + name,
		expected, line);
}

void TestInstance::WaitOutputLine(const string &expected) {
	for (list<string>::iterator i(past_out.begin()); i != past_out.end(); ++i) {
		if (*i == expected) {
			past_out.erase(i);
			return;
		}
	}
	string line;
	while (true) {
		ReadOutputLine(line);
		if (line == expected) {
			return;
		} else {
			past_out.push_back(line);
		}
	}
}

void TestInstance::ExhaustOutput(string &output) {
	output.clear();
	for (const auto &line : past_out) {
		output += line;
		output += '\n';
	}
	past_out.clear();
	string line;
	while (true) {
		if (out_buf.Extract(line)) {
			output += line;
			output += '\n';
		} else {
			// buffer exhausted, fetch more data
			int len = proc.ReadOut(out_buf.WriteHead(), out_buf.Remain(), 10000);
			if (len == 0) {
				// eof
				return;
			}
			out_buf.Update(len);
		}
	}
}

void TestInstance::AssertNoOutput() {
	string output;
	ExhaustOutput(output);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected output of test instance " + name,
		string(""), output);
}


void TestInstance::ReadErrorLine(string &line) {
	while (!err_buf.Extract(line)) {
		// buffer exhausted, fetch more data
		int len = proc.ReadErr(err_buf.WriteHead(), err_buf.Remain(), 10000);
		if (len == 0) {
			throw runtime_error("failed read from stderr of " + name);
		}
		err_buf.Update(len);
	}
}

void TestInstance::AssertErrorLine(const string &expected) {
	string line;
	if (past_err.empty()) {
		ReadErrorLine(line);
	} else {
		line = past_err.front();
		past_err.pop_front();
	}
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected line in stderr",
		expected, line);
}

void TestInstance::WaitErrorLine(const string &expected) {
	for (list<string>::iterator i(past_err.begin()); i != past_err.end(); ++i) {
		if (*i == expected) {
			past_err.erase(i);
			return;
		}
	}
	string line;
	while (true) {
		ReadErrorLine(line);
		if (line == expected) {
			return;
		} else {
			past_err.push_back(line);
		}
	}
}

void TestInstance::ExhaustError(string &error) {
	error.clear();
	for (const auto &line : past_err) {
		error += line;
		error += '\n';
	}
	past_err.clear();
	string line;
	while (true) {
		if (err_buf.Extract(line)) {
			error += line;
			error += '\n';
		} else {
			// buffer exhausted, fetch more data
			int len = proc.ReadErr(err_buf.WriteHead(), err_buf.Remain(), 10000);
			if (len == 0) {
				// eof
				return;
			}
			err_buf.Update(len);
		}
	}
}

void TestInstance::AssertNoError() {
	string error;
	ExhaustError(error);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected error output of test instance " + name,
		string(""), error);
}


void TestInstance::Terminate() {
	if (!proc.Terminated()) {
		proc.Terminate();
	}
}

void TestInstance::AssertRunning() {
	CPPUNIT_ASSERT_MESSAGE(
		"test instance " + name + " terminated unexpectedly",
		!proc.Terminated());
}

void TestInstance::AssertTerminated() {
	CPPUNIT_ASSERT_MESSAGE(
		"test instance " + name + " did not terminate as expected",
		proc.Terminated());
}

void TestInstance::AssertExitStatus(int expected) {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected exit status from child program " + name,
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
