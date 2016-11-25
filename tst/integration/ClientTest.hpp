#ifndef BLANK_TEST_INTEGRATION_CLIENTTEST_HPP_
#define BLANK_TEST_INTEGRATION_CLIENTTEST_HPP_

#include <memory>
#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class TestInstance;

class ClientTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(ClientTest);

CPPUNIT_TEST(testStartup);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testStartup();

private:
	std::unique_ptr<TestInstance> server;
	std::unique_ptr<TestInstance> client;

};

}
}

#endif
