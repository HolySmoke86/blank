#ifndef BLANK_TEST_INTEGRATION_SERVERTEST_HPP_
#define BLANK_TEST_INTEGRATION_SERVERTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class ServerTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(ServerTest);

CPPUNIT_TEST(testStartup);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testStartup();

};

}
}

#endif
