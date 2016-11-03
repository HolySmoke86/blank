#ifndef BLANK_TEST_GEOMETRY_LOCATIONTEST_HPP_
#define BLANK_TEST_GEOMETRY_LOCATIONTEST_HPP_

#include "geometry/Location.hpp"
#include "graphics/glm.hpp"

#include <string>
#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class LocationTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(LocationTest);

CPPUNIT_TEST(testSanitize);
CPPUNIT_TEST(testAbsolute);
CPPUNIT_TEST(testRelative);
CPPUNIT_TEST(testDifference);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testSanitize();
	void testAbsolute();
	void testRelative();
	void testDifference();

private:
	static void AssertEqual(
		const std::string &msg,
		const glm::ivec3 &expected,
		const glm::ivec3 &actual);
	static void AssertEqual(
		const std::string &msg,
		const glm::vec3 &expected,
		const glm::vec3 &actual);
	static void AssertEqual(
		const std::string &msg,
		const RoughLocation &expected,
		const RoughLocation &actual);
	static void AssertEqual(
		const std::string &msg,
		const ExactLocation &expected,
		const ExactLocation &actual);

};

}
}

#endif
