#include "StabilityTest.hpp"

#include "rand/GaloisLFSR.hpp"
#include "rand/SimplexNoise.hpp"

#include <cstdint>
#include <string>
#include <sstream>
#include <glm/gtx/io.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::StabilityTest);

using namespace std;


namespace blank {
namespace test {

void StabilityTest::setUp() {

}

void StabilityTest::tearDown() {

}


void StabilityTest::testRNG() {
	GaloisLFSR random(0);
	uint16_t value;
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #1 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #2 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #3 from RNG",
		uint16_t(0xB000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #4 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #5 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #6 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #7 from RNG",
		uint16_t(0x4500), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #8 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #9 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #10 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #11 from RNG",
		uint16_t(0x2E70), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #12 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #13 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #14 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #15 from RNG",
		uint16_t(0x1011), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #16 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #17 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #18 from RNG",
		uint16_t(0xB000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #19 from RNG",
		uint16_t(0x0B0B), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #20 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #21 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #22 from RNG",
		uint16_t(0x1500), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #23 from RNG",
		uint16_t(0x0454), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #24 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #25 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #26 from RNG",
		uint16_t(0xC970), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #27 from RNG",
		uint16_t(0x02E5), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #28 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #29 from RNG",
		uint16_t(0x0000), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #30 from RNG",
		uint16_t(0x0101), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #31 from RNG",
		uint16_t(0x0100), value
	);
	random(value);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected value #32 from RNG",
		uint16_t(0x0000), value
	);

	GaloisLFSR random1(1);
	uint16_t value1;
	for (int i = 0; i < 32; ++i) {
		random1(value1);
	}
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"RNG with seeds 0 and 1 differ",
		value, value1
	);
}

void StabilityTest::testSimplex() {
	SimplexNoise noise(0);

	Assert(noise, glm::vec3(0.0f, 0.0f, 0.0f),  0.0f);
	Assert(noise, glm::vec3(0.0f, 0.0f, 1.0f),  0.652221322059631f);
	Assert(noise, glm::vec3(0.0f, 1.0f, 0.0f),  0.867977976799011f);
	Assert(noise, glm::vec3(0.0f, 1.0f, 1.0f), -0.107878111302853f);
	Assert(noise, glm::vec3(1.0f, 0.0f, 0.0f), -0.107878260314465f);
	Assert(noise, glm::vec3(1.0f, 0.0f, 1.0f), -6.31356940061778e-08f);
	Assert(noise, glm::vec3(1.0f, 1.0f, 0.0f), -0.107878245413303f);
	Assert(noise, glm::vec3(1.0f, 1.0f, 1.0f),  0.0f);

	Assert(noise, glm::vec3( 0.0f,  0.0f, -1.0f), -0.107878483831882f);
	Assert(noise, glm::vec3( 0.0f, -1.0f,  0.0f), -0.760099768638611f);
	Assert(noise, glm::vec3( 0.0f, -1.0f, -1.0f),  0.0f);
	Assert(noise, glm::vec3(-1.0f,  0.0f,  0.0f),  0.760099768638611f);
	Assert(noise, glm::vec3(-1.0f,  0.0f, -1.0f),  0.0f);
	Assert(noise, glm::vec3(-1.0f, -1.0f,  0.0f), -0.107878118753433f);
	Assert(noise, glm::vec3(-1.0f, -1.0f, -1.0f),  0.0f);
}

void StabilityTest::Assert(
	const SimplexNoise &noise,
	const glm::vec3 &position,
	float expected
) {
	stringstream msg;
	msg << "unexpected simplex noise value at " << position;
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg.str(),
		expected, noise(position), numeric_limits<float>::epsilon()
	);
}

}
}
