#ifndef BLANK_TEST_RAND_STABILITYTEST_HPP
#define BLANK_TEST_RAND_STABILITYTEST_HPP

#include "graphics/glm.hpp"

#include <cppunit/extensions/HelperMacros.h>



namespace blank {

class SimplexNoise;
class WorleyNoise;

namespace test {

class StabilityTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(StabilityTest);

CPPUNIT_TEST(testRNG);
CPPUNIT_TEST(testSimplex);
CPPUNIT_TEST(testWorley);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testRNG();
	void testSimplex();
	void testWorley();

	static void Assert(
		const SimplexNoise &noise,
		const glm::vec3 &position,
		float expected);

	static void Assert(
		const WorleyNoise &noise,
		const glm::vec3 &position,
		float expected);

};

}
}

#endif
