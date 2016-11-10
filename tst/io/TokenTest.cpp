#include "TokenTest.hpp"

#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::TokenTest);

using namespace std;

namespace blank {
namespace test {

void TokenTest::setUp() {

}

void TokenTest::tearDown() {

}


void TokenTest::testTypeIO() {
	AssertStreamOutput(Token::UNKNOWN, "UNKNOWN");
	AssertStreamOutput(Token::ANGLE_BRACKET_OPEN, "ANGLE_BRACKET_OPEN");
	AssertStreamOutput(Token::ANGLE_BRACKET_CLOSE, "ANGLE_BRACKET_CLOSE");
	AssertStreamOutput(Token::CHEVRON_OPEN, "CHEVRON_OPEN");
	AssertStreamOutput(Token::CHEVRON_CLOSE, "CHEVRON_CLOSE");
	AssertStreamOutput(Token::BRACKET_OPEN, "BRACKET_OPEN");
	AssertStreamOutput(Token::BRACKET_CLOSE, "BRACKET_CLOSE");
	AssertStreamOutput(Token::PARENTHESIS_OPEN, "PARENTHESIS_OPEN");
	AssertStreamOutput(Token::PARENTHESIS_CLOSE, "PARENTHESIS_CLOSE");
	AssertStreamOutput(Token::COLON, "COLON");
	AssertStreamOutput(Token::SEMICOLON, "SEMICOLON");
	AssertStreamOutput(Token::COMMA, "COMMA");
	AssertStreamOutput(Token::EQUALS, "EQUALS");
	AssertStreamOutput(Token::NUMBER, "NUMBER");
	AssertStreamOutput(Token::STRING, "STRING");
	AssertStreamOutput(Token::IDENTIFIER, "IDENTIFIER");
	AssertStreamOutput(Token::COMMENT, "COMMENT");
}

void TokenTest::testTokenIO() {
	Token t;
	t.value = "why oh why";
	AssertStreamOutput(t, "UNKNOWN(why oh why)");
	t.type = Token::UNKNOWN;
	t.value = "do I have no purpose";
	AssertStreamOutput(t, "UNKNOWN(do I have no purpose)");
	t.type = Token::ANGLE_BRACKET_OPEN;
	AssertStreamOutput(t, "ANGLE_BRACKET_OPEN");
	t.type = Token::ANGLE_BRACKET_CLOSE;
	AssertStreamOutput(t, "ANGLE_BRACKET_CLOSE");
	t.type = Token::CHEVRON_OPEN;
	AssertStreamOutput(t, "CHEVRON_OPEN");
	t.type = Token::CHEVRON_CLOSE;
	AssertStreamOutput(t, "CHEVRON_CLOSE");
	t.type = Token::BRACKET_OPEN;
	AssertStreamOutput(t, "BRACKET_OPEN");
	t.type = Token::BRACKET_CLOSE;
	AssertStreamOutput(t, "BRACKET_CLOSE");
	t.type = Token::PARENTHESIS_OPEN;
	AssertStreamOutput(t, "PARENTHESIS_OPEN");
	t.type = Token::PARENTHESIS_CLOSE;
	AssertStreamOutput(t, "PARENTHESIS_CLOSE");
	t.type = Token::COLON;
	AssertStreamOutput(t, "COLON");
	t.type = Token::SEMICOLON;
	AssertStreamOutput(t, "SEMICOLON");
	t.type = Token::COMMA;
	AssertStreamOutput(t, "COMMA");
	t.type = Token::EQUALS;
	AssertStreamOutput(t, "EQUALS");
	t.type = Token::NUMBER;
	t.value = "15";
	AssertStreamOutput(t, "NUMBER(15)");
	t.type = Token::STRING;
	t.value = "hello world";
	AssertStreamOutput(t, "STRING(hello world)");
	t.type = Token::IDENTIFIER;
	t.value = "foo";
	AssertStreamOutput(t, "IDENTIFIER(foo)");
	t.type = Token::COMMENT;
	t.value = "WITHOUT ANY WARRANTY";
	AssertStreamOutput(t, "COMMENT(WITHOUT ANY WARRANTY)");
}


void TokenTest::AssertStreamOutput(
	Token::Type t,
	string expected
) {
	stringstream conv;
	conv << t;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected std::ostream << Token::Type result",
		expected, conv.str());
}

void TokenTest::AssertStreamOutput(
	const Token &t,
	string expected
) {
	stringstream conv;
	conv << t;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected std::ostream << Token result",
		expected, conv.str());
}

}
}
