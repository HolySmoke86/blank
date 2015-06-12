#ifndef BLANK_TEST_APP_TIMERTEST_H_
#define BLANK_TEST_APP_TIMERTEST_H_

#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class TimerTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(TimerTest);

CPPUNIT_TEST(testIntervalTimer);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testIntervalTimer();

};

}
}

#endif
