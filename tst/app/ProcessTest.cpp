#include "ProcessTest.hpp"

#include "app/Process.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::ProcessTest);

using namespace std;


namespace blank {
namespace test {

void ProcessTest::setUp() {

}

void ProcessTest::tearDown() {

}


void ProcessTest::testExit() {
#ifdef __WIN32
#  error "TODO: implement Process tests for windows"
#else

	{
		Process proc("/usr/bin/env", { "env", "true" });
		int status = proc.Join();
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"exit status of true assumed 0",
			0, status);
	}

	{
		Process proc("/usr/bin/env", { "env", "false" });
		int status = proc.Join();
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"exit status of false assumed 1",
			1, status);
	}

#endif
}

void ProcessTest::testStream() {
#ifdef __WIN32
#  error "TODO: implement Process tests for windows"
#else

	{
		const string test_input("hello, world");
		const string expected_output("hello, world\n");
		Process proc("/usr/bin/env", { "env", "echo", test_input.c_str() });
		char buffer[expected_output.length() + 1];
		size_t len = proc.ReadOut(buffer, sizeof(buffer), 1000);
		const string output(buffer, len);
		int status = proc.Join();
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"exit status of echo assumed 0",
			0, status);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"unexpected length of echo output",
			expected_output.size(), len);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"unexpected output of echo",
			expected_output, output);
	}

	{
		const string test_input("hello, error");
		const string expected_output("hello, error\n");
		Process proc("/usr/bin/env", { "env", "sh", "-c", "echo $1 >&2", "echo", test_input.c_str() }, { });
		char buffer[expected_output.length() + 1];
		size_t len = proc.ReadErr(buffer, sizeof(buffer), 1000);
		const string output(buffer, len);
		int status = proc.Join();
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"exit status of echo assumed 0",
			0, status);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"unexpected length of echo output",
			expected_output.size(), len);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"unexpected error output of echo",
			expected_output, output);
	}


	{
		const string test_input("dog");
		const string expected_output("dog");
		Process proc("/usr/bin/env", { "env", "cat" });
		size_t len = proc.WriteIn(test_input.c_str(), test_input.size());
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"unexpected length of input to cat",
			test_input.size(), len);
		// close input stream so cat knows we're done
		proc.CloseIn();

		char buffer[expected_output.length() + 1];
		len = proc.ReadOut(buffer, sizeof(buffer), 1000);
		const string output(buffer, len);
		int status = proc.Join();
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"exit status of cat assumed 0",
			0, status);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"unexpected length of cat output",
			expected_output.size(), len);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"unexpected output of cat",
			expected_output, output);
	}

#endif
}


void ProcessTest::testEnv() {
#ifdef __WIN32
#  error "TODO: implement Process tests for windows"
#else
	{
		const string test_input("Hello, environment");
		const string expected_output("Hello, environment\n");
		Process proc("/usr/bin/env", { "env", "sh", "-c", "echo $BLANK_ENV_TEST" }, { "BLANK_ENV_TEST=" + test_input });
		char buffer[expected_output.length() + 1];
		size_t len = proc.ReadOut(buffer, sizeof(buffer), 1000);
		const string output(buffer, len);
		int status = proc.Join();
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"exit status of echo assumed 0",
			0, status);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"unexpected length of echo output",
			expected_output.size(), len);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"unexpected error output of echo",
			expected_output, output);
	}

#endif
}

void ProcessTest::testTimeout() {
#ifdef __WIN32
#  error "TODO: implement Process tests for windows"
#else
	Process proc("/usr/bin/env", { "env", "cat" });
	char buffer;
	CPPUNIT_ASSERT_THROW_MESSAGE(
		"read timeout on child process' stdout should throw",
		proc.ReadOut(&buffer, 1, 1), std::runtime_error);
	CPPUNIT_ASSERT_THROW_MESSAGE(
		"read timeout on child process' stderr should throw",
		proc.ReadErr(&buffer, 1, 1), std::runtime_error);
	proc.Terminate();
#endif
}

}
}
