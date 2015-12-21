#ifndef BLANK_TEST_GEOMETRY_INTERSECTIONTEST_H_
#define BLANK_TEST_GEOMETRY_INTERSECTIONTEST_H_

#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class IntersectionTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(IntersectionTest);

CPPUNIT_TEST(testSimpleRayBoxIntersection);
CPPUNIT_TEST(testRayBoxIntersection);
CPPUNIT_TEST(testBoxBoxIntersection);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testSimpleRayBoxIntersection();
	void testRayBoxIntersection();
	void testBoxBoxIntersection();

};

}
}

#endif
