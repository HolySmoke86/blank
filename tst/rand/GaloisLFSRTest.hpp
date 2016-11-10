#ifndef BLANK_TEST_RAND_GALOISLFSRTEST_HPP
#define BLANK_TEST_RAND_GALOISLFSRTEST_HPP

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <vector>


namespace blank {

namespace test {

class GaloisLFSRTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(GaloisLFSRTest);

CPPUNIT_TEST(testFloatNorm);
CPPUNIT_TEST(testFromContainer);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testFloatNorm();
	void testFromContainer();

	/// check if value is in range [minimum,maximum]
	static void AssertBetween(
		std::string message,
		float minimum,
		float maximum,
		float actual);

	static void AssertContains(
		std::string message,
		const std::vector<int> &container,
		int element);

};

}
}

#endif
