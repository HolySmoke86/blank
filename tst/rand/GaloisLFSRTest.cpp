#include "GaloisLFSRTest.hpp"

#include "rand/GaloisLFSR.hpp"

#include <algorithm>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::GaloisLFSRTest);

using namespace std;


namespace blank {
namespace test {

void GaloisLFSRTest::setUp() {

}

void GaloisLFSRTest::tearDown() {

}

void GaloisLFSRTest::testFloatNorm() {
	GaloisLFSR random(4);
	for (int i = 0; i < 64; ++i) {
		float value = random.SNorm();
		AssertBetween(
			"random signed normal float",
			-1.0f, 1.0f, value);
	}
	for (int i = 0; i < 64; ++i) {
		float value = random.UNorm();
		AssertBetween(
			"random unsigned normal float",
			0.0f, 1.0f, value);
	}
}

void GaloisLFSRTest::testFromContainer() {
	GaloisLFSR random(5);
	const vector<int> container({ 1, 2, 3, 4, 5 });
	for (int i = 0; i < 64; ++i) {
		int element = random.From(container);
		AssertContains(
			"random element from container",
			container, element);
	}
}

void GaloisLFSRTest::AssertBetween(
	string message,
	float minimum,
	float maximum,
	float actual
) {
	stringstream msg;
	msg << message << ": " << actual << " not in ["
		<< minimum << ',' << maximum << ']';
	CPPUNIT_ASSERT_MESSAGE(
		msg.str(),
		minimum <= actual && actual <= maximum);

}

void GaloisLFSRTest::AssertContains(
	string message,
	const vector<int> &container,
	int element
) {
	stringstream msg;
	msg << message << ": " << element << " not in { ";
	for (int i : container) {
		msg << i << ' ';
	}
	msg << '}';
	CPPUNIT_ASSERT_MESSAGE(
		msg.str(),
		find(container.begin(), container.end(), element) != container.end());

}

}
}
