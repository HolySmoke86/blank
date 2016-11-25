#ifndef BLANK_TEST_INTEGRATION_STANDALONETEST_HPP_
#define BLANK_TEST_INTEGRATION_STANDALONETEST_HPP_

#include <memory>
#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class TestInstance;

class StandaloneTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(StandaloneTest);

CPPUNIT_TEST(testStartup);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testStartup();

private:
	std::unique_ptr<TestInstance> instance;

};

}
}

#endif
