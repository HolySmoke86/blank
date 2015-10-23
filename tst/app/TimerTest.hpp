#ifndef BLANK_TEST_APP_TIMERTEST_H_
#define BLANK_TEST_APP_TIMERTEST_H_

#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class TimerTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(TimerTest);

CPPUNIT_TEST(testCoarseTimer);
CPPUNIT_TEST(testFineTimer);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testCoarseTimer();
	void testFineTimer();

};

}
}

#endif
