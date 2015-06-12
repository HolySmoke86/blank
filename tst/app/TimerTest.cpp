#include "TimerTest.hpp"

#include "app/IntervalTimer.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::TimerTest);


namespace blank {
namespace test {

void TimerTest::setUp() {
}

void TimerTest::tearDown() {
}


void TimerTest::testIntervalTimer() {
	IntervalTimer timer(50);
	CPPUNIT_ASSERT_MESSAGE(
		"fresh timer is running",
		!timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"fresh timer hit",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"fresh timer with non-zero elapsed time",
		0, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"fresh timer at non-zero iteration",
		0, timer.Iteration()
	);

	timer.Start();
	CPPUNIT_ASSERT_MESSAGE(
		"startet timer is not running",
		timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"started timer hit without update",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"started, but not updated timer with non-zero elapsed time",
		0, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"started, but not updated timer at non-zero iteration",
		0, timer.Iteration()
	);

	timer.Update(25);
	CPPUNIT_ASSERT_MESSAGE(
		"updated timer is not running",
		timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"timer hit after update, but before it should",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong elapsed time on updated timer",
		25, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration on updated timer",
		0, timer.Iteration()
	);

	timer.Update(25);
	CPPUNIT_ASSERT_MESSAGE(
		"timer not hit after updating to its exact interval time",
		timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong elapsed time on updated timer",
		50, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration on updated timer at exact interval time",
		1, timer.Iteration()
	);

	timer.Update(49);
	CPPUNIT_ASSERT_MESSAGE(
		"timer hit after updating from exact interval time to just before the next",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong elapsed time on updated timer",
		99, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration after updating timer from exact interval time to just before the next",
		1, timer.Iteration()
	);

	timer.Update(2);
	CPPUNIT_ASSERT_MESSAGE(
		"timer not hit after updating across interval time boundary",
		timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong elapsed time on updated timer",
		101, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration after updating across interval time boundary",
		2, timer.Iteration()
	);

	timer.Stop();
	CPPUNIT_ASSERT_MESSAGE(
		"stopped timer is running",
		!timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"stopped timer hit",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"stopped timer has non-zero elapsed time",
		0, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"stopped timer at non-zero iteration",
		0, timer.Iteration()
	);
}

}
}
