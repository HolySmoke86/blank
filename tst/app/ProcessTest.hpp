#ifndef BLANK_TEST_APP_PROCESSTEST_HPP_
#define BLANK_TEST_APP_PROCESSTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class ProcessTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(ProcessTest);

CPPUNIT_TEST(testExit);
CPPUNIT_TEST(testStream);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testExit();
	void testStream();

};

}
}

#endif
