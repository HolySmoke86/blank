#ifndef BLANK_TEST_IO_EVENTTEST_HPP
#define BLANK_TEST_IO_EVENTTEST_HPP

#include <cppunit/extensions/HelperMacros.h>

#include <SDL_version.h>

namespace blank {
namespace test {

class EventTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(EventTest);

#if SDL_VERSION_ATLEAST(2, 0, 4)
CPPUNIT_TEST(testAudioDevice);
#endif

CPPUNIT_TEST(testController);
CPPUNIT_TEST(testDollar);
CPPUNIT_TEST(testDrop);
CPPUNIT_TEST(testFinger);
CPPUNIT_TEST(testKey);
CPPUNIT_TEST(testJoystick);
CPPUNIT_TEST(testMouse);
CPPUNIT_TEST(testMultiGesture);
CPPUNIT_TEST(testQuit);
CPPUNIT_TEST(testSysWM);
CPPUNIT_TEST(testText);
CPPUNIT_TEST(testUser);
CPPUNIT_TEST(testWindow);
CPPUNIT_TEST(testUnknown);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

#if SDL_VERSION_ATLEAST(2, 0, 4)
	void testAudioDevice();
#endif

	void testController();
	void testDollar();
	void testDrop();
	void testFinger();
	void testKey();
	void testJoystick();
	void testMouse();
	void testMultiGesture();
	void testQuit();
	void testSysWM();
	void testText();
	void testUser();
	void testWindow();
	void testUnknown();

};

}
}


#endif
