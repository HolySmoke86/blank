#ifndef BLANK_TEST_WORLD_BLOCKTEST_H_
#define BLANK_TEST_WORLD_BLOCKTEST_H_

#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class BlockTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(BlockTest);

CPPUNIT_TEST(testOrientation);
CPPUNIT_TEST(testFaceOpposite);
CPPUNIT_TEST(testFaceAxis);
CPPUNIT_TEST(testFaceNormal);
CPPUNIT_TEST(testNormalFace);

CPPUNIT_TEST(testFaceSet);

CPPUNIT_TEST(testDefaultBlock);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testOrientation();

	void testFaceOpposite();
	void testFaceAxis();
	void testFaceNormal();
	void testNormalFace();

	void testFaceSet();

	void testDefaultBlock();

};

}
}

#endif
