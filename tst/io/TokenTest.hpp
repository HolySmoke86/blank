#ifndef BLANK_TEST_IO_TOKENTEST_HPP
#define BLANK_TEST_IO_TOKENTEST_HPP

#include "io/Token.hpp"
#include "io/Tokenizer.hpp"

#include <string>
#include <cppunit/extensions/HelperMacros.h>


namespace blank {

namespace test {

class TokenTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(TokenTest);

CPPUNIT_TEST(testTypeIO);
CPPUNIT_TEST(testTokenIO);
CPPUNIT_TEST(testTokenizer);
CPPUNIT_TEST(testTokenizerBrokenComment);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testTypeIO();
	void testTokenIO();
	void testTokenizer();
	void testTokenizerBrokenComment();

	static void AssertStreamOutput(
		Token::Type, std::string expected);
	static void AssertStreamOutput(
		const Token &, std::string expected);

	static void AssertHasMore(Tokenizer &);
	static void AssertToken(
		Token::Type expected_type, const Token &actual_token);
	static void AssertToken(
		Token::Type expected_type, std::string expected_value,
		const Token &actual_token);

};

}
}

#endif
