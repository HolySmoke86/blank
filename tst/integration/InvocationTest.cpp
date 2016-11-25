#include "InvocationTest.hpp"

#include "TestInstance.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::InvocationTest);


namespace blank {
namespace test {

void InvocationTest::setUp() {

}

void InvocationTest::tearDown() {

}


void InvocationTest::testUnknownArg() {
	TestInstance prog({ "--worscht", "-W", "käs" });
	prog.AssertErrorLine("unknown option --worscht");
	prog.AssertErrorLine("unknown option W");
	prog.AssertErrorLine("unable to interpret argument 3 (käs)");
	prog.AssertExitStatus(1);
	prog.AssertNoError();
	prog.AssertNoOutput();
}

void InvocationTest::testIncompleteOption() {
	TestInstance prog({
		"-",
		"",
		"--",
		"-"
	}, false, false);
	prog.AssertErrorLine("warning: incomplete option list at position 1");
	prog.AssertErrorLine("warning: found empty argument at position 2");
	prog.AssertErrorLine("unable to interpret argument 4 (-)");
	prog.AssertExitStatus(1);
	prog.AssertNoError();
	prog.AssertNoOutput();
}

void InvocationTest::testMissingArgs() {
	const std::vector<std::string> opts_with_args = {
		"--asset-path",
		"--host",
		"--port",
		"--cmd-port",
		"--player-name",
		"--save-path",
		"--world-name",
		"-m",
		"-n",
		"-s",
		"-t",
	};
	for (auto arg : opts_with_args) {
		TestInstance prog({ arg }, false, false);
		prog.AssertErrorLine("missing argument to " + arg);
		prog.AssertExitStatus(1);
		prog.AssertNoError();
		prog.AssertNoOutput();
	}
}

}
}
