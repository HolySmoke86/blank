#ifndef BLANK_TEST_GRAPHICS_GLTRAITSTEST_HPP_
#define BLANK_TEST_GRAPHICS_GLTRAITSTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>

namespace blank {
namespace test {

class GLTraitsTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(GLTraitsTest);

CPPUNIT_TEST(testSize);
CPPUNIT_TEST(testType);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testSize();
	void testType();

};

}
}

#endif
