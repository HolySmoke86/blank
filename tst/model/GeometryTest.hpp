#ifndef BLANK_TEST_MODEL_GEOMETRYTEST_H_
#define BLANK_TEST_MODEL_GEOMETRYTEST_H_

#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class GeometryTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(GeometryTest);

CPPUNIT_TEST(testRayBoxIntersection);
CPPUNIT_TEST(testBoxBoxIntersection);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testRayBoxIntersection();
	void testBoxBoxIntersection();

};

}
}

#endif
