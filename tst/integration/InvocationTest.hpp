#ifndef BLANK_TEST_INTEGRATION_INVOCATIONTEST_HPP_
#define BLANK_TEST_INTEGRATION_INVOCATIONTEST_HPP_

#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class InvocationTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(InvocationTest);

CPPUNIT_TEST(testUnknownArg);
CPPUNIT_TEST(testIncompleteOption);
CPPUNIT_TEST(testMissingArgs);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testUnknownArg();
	void testIncompleteOption();
	void testMissingArgs();

};

}
}

#endif
