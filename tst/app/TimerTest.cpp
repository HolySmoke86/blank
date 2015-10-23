#include "TimerTest.hpp"

#include "app/IntervalTimer.hpp"

#include <limits>

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::TimerTest);


namespace blank {
namespace test {

void TimerTest::setUp() {
}

void TimerTest::tearDown() {
}


void TimerTest::testCoarseTimer() {
	CoarseTimer timer(50);
	CPPUNIT_ASSERT_MESSAGE(
		"fresh coarse timer is running",
		!timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"fresh coarse timer hit",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"fresh coarse timer with non-zero elapsed time",
		0, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"fresh coarse timer at non-zero iteration",
		0, timer.Iteration()
	);

	timer.Start();
	CPPUNIT_ASSERT_MESSAGE(
		"startet coarse timer is not running",
		timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"started coarse timer hit without update",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"started, but not updated coarse timer with non-zero elapsed time",
		0, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"started, but not updated coarse timer at non-zero iteration",
		0, timer.Iteration()
	);

	timer.Update(25);
	CPPUNIT_ASSERT_MESSAGE(
		"updated coarse timer is not running",
		timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"coarse timer hit after update, but before it should",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong elapsed time on updated coarse timer",
		25, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration on updated coarse timer",
		0, timer.Iteration()
	);

	timer.Update(25);
	CPPUNIT_ASSERT_MESSAGE(
		"coarse timer not hit after updating to its exact interval time",
		timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong elapsed time on updated coarse timer",
		50, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration on updated coarse timer at exact interval time",
		1, timer.Iteration()
	);

	timer.Update(49);
	CPPUNIT_ASSERT_MESSAGE(
		"coarse timer hit after updating from exact interval time to just before the next",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong elapsed time on updated coarse timer",
		99, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration after updating coarse timer from exact interval time to just before the next",
		1, timer.Iteration()
	);

	timer.Update(2);
	CPPUNIT_ASSERT_MESSAGE(
		"coarse timer not hit after updating across interval time boundary",
		timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong elapsed time on updated coarse timer",
		101, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration after updating across interval time boundary",
		2, timer.Iteration()
	);

	timer.Stop();
	CPPUNIT_ASSERT_MESSAGE(
		"stopped coarse timer is running",
		!timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"stopped coarse timer hit",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"stopped coarse timer has non-zero elapsed time",
		0, timer.Elapsed()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"stopped coarse timer at non-zero iteration",
		0, timer.Iteration()
	);
}

void TimerTest::testFineTimer() {
	FineTimer timer(0.5f);
	CPPUNIT_ASSERT_MESSAGE(
		"fresh fine timer is running",
		!timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"fresh fine timer hit",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"fresh fine timer with non-zero elapsed time",
		0.0f, timer.Elapsed(), std::numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"fresh fine timer at non-zero iteration",
		0, timer.Iteration()
	);

	timer.Start();
	CPPUNIT_ASSERT_MESSAGE(
		"startet fine timer is not running",
		timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"started fine timer hit without update",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"started, but not updated fine timer with non-zero elapsed time",
		0.0f, timer.Elapsed(), std::numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"started, but not updated fine timer at non-zero iteration",
		0, timer.Iteration()
	);

	timer.Update(0.25f);
	CPPUNIT_ASSERT_MESSAGE(
		"updated fine timer is not running",
		timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"fine timer hit after update, but before it should",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong elapsed time on updated fine timer",
		0.25f, timer.Elapsed(), std::numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration on updated fine timer",
		0, timer.Iteration()
	);

	timer.Update(0.25f);
	CPPUNIT_ASSERT_MESSAGE(
		"fine timer not hit after updating to its exact interval time",
		timer.Hit()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong elapsed time on updated fine timer",
		0.5f, timer.Elapsed(), std::numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration on updated fine timer at exact interval time",
		1, timer.Iteration()
	);

	timer.Update(0.49f);
	CPPUNIT_ASSERT_MESSAGE(
		"fine timer hit after updating from exact interval time to just before the next",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong elapsed time on updated fine timer",
		0.99f, timer.Elapsed(), std::numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration after updating fine timer from exact interval time to just before the next",
		1, timer.Iteration()
	);

	timer.Update(0.02f);
	CPPUNIT_ASSERT_MESSAGE(
		"fine timer not hit after updating across interval time boundary",
		timer.Hit()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"wrong elapsed time on updated fine timer",
		1.01f, timer.Elapsed(), std::numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong iteration after updating across interval time boundary",
		2, timer.Iteration()
	);

	timer.Stop();
	CPPUNIT_ASSERT_MESSAGE(
		"stopped fine timer is running",
		!timer.Running()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"stopped fine timer hit",
		!timer.Hit()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"stopped fine timer has non-zero elapsed time",
		0.0f, timer.Elapsed(), std::numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"stopped fine timer at non-zero iteration",
		0, timer.Iteration()
	);
}

}
}
