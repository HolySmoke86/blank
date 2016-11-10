#ifndef BLANK_TEST_IO_TOKENTEST_HPP
#define BLANK_TEST_IO_TOKENTEST_HPP

#include "io/Token.hpp"

#include <string>
#include <cppunit/extensions/HelperMacros.h>


namespace blank {

namespace test {

class TokenTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(TokenTest);

CPPUNIT_TEST(testTypeIO);
CPPUNIT_TEST(testTokenIO);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testTypeIO();
	void testTokenIO();

	static void AssertStreamOutput(
		Token::Type, std::string expected);
	static void AssertStreamOutput(
		const Token &, std::string expected);

};

}
}

#endif
