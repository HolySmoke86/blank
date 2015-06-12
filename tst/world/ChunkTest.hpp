#ifndef BLANK_TEST_WORLD_CHUNKTEST_H_
#define BLANK_TEST_WORLD_CHUNKTEST_H_

#include "world/BlockTypeRegistry.hpp"

#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class ChunkTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(ChunkTest);

CPPUNIT_TEST(testBounds);
CPPUNIT_TEST(testBorder);
CPPUNIT_TEST(testNeighbor);

CPPUNIT_TEST(testBlock);
CPPUNIT_TEST(testLight);
CPPUNIT_TEST(testLightPropagation);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testBounds();
	void testBorder();
	void testNeighbor();

	void testBlock();
	void testLight();
	void testLightPropagation();

private:
	BlockTypeRegistry types;

};

}
}

#endif
