#include "TokenTest.hpp"

#include <sstream>
#include <stdexcept>

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

void TokenTest::testTokenizer() {
	stringstream stream;
	stream << "[{0},<.5>+3=/**\n * test\n */ (-1.5); foo_bar.baz:\"hello\\r\\n\\t\\\"world\\\"\" ] // this line\n#that line";
	Tokenizer in(stream);

	AssertHasMore(in);
	Token token(in.Next());
	AssertToken(token.type, token.value, in.Current());
	AssertToken(Token::BRACKET_OPEN, token);

	AssertHasMore(in);
	AssertToken(Token::ANGLE_BRACKET_OPEN, in.Next());
	AssertHasMore(in);
	AssertToken(Token::NUMBER, "0", in.Next());
	AssertHasMore(in);
	AssertToken(Token::ANGLE_BRACKET_CLOSE, in.Next());
	AssertHasMore(in);
	AssertToken(Token::COMMA, in.Next());
	AssertHasMore(in);
	AssertToken(Token::CHEVRON_OPEN, in.Next());
	AssertHasMore(in);
	AssertToken(Token::NUMBER, ".5", in.Next());
	AssertHasMore(in);
	AssertToken(Token::CHEVRON_CLOSE, in.Next());
	AssertHasMore(in);
	AssertToken(Token::NUMBER, "+3", in.Next());
	AssertHasMore(in);
	AssertToken(Token::EQUALS, in.Next());
	AssertHasMore(in);
	AssertToken(Token::COMMENT, "*\n * test\n ", in.Next());
	AssertHasMore(in);
	AssertToken(Token::PARENTHESIS_OPEN, in.Next());
	AssertHasMore(in);
	AssertToken(Token::NUMBER, "-1.5", in.Next());
	AssertHasMore(in);
	AssertToken(Token::PARENTHESIS_CLOSE, in.Next());
	AssertHasMore(in);
	AssertToken(Token::SEMICOLON, in.Next());
	AssertHasMore(in);
	AssertToken(Token::IDENTIFIER, "foo_bar.baz", in.Next());
	AssertHasMore(in);
	AssertToken(Token::COLON, in.Next());
	AssertHasMore(in);
	AssertToken(Token::STRING, "hello\r\n\t\"world\"", in.Next());
	AssertHasMore(in);
	AssertToken(Token::BRACKET_CLOSE, in.Next());
	AssertHasMore(in);
	AssertToken(Token::COMMENT, " this line", in.Next());
	AssertHasMore(in);
	AssertToken(Token::COMMENT, "that line", in.Next());
	CPPUNIT_ASSERT_MESSAGE("expected end of stream", !in.HasMore());
}

void TokenTest::testTokenizerBrokenComment() {
	{
		stringstream stream;
		stream << "/* just one more thing…*";
		Tokenizer in(stream);
		AssertHasMore(in);
		CPPUNIT_ASSERT_THROW_MESSAGE(
			"half-closed comment should throw",
			in.Next(), std::runtime_error);
	}
	{
		stringstream stream;
		stream << "  /";
		Tokenizer in(stream);
		AssertHasMore(in);
		CPPUNIT_ASSERT_THROW_MESSAGE(
			"sole '/' at end of stream should throw",
			in.Next(), std::runtime_error);
	}
	{
		stringstream stream;
		stream << "/.";
		Tokenizer in(stream);
		AssertHasMore(in);
		CPPUNIT_ASSERT_THROW_MESSAGE(
			"'/' followed by garbage should throw",
			in.Next(), std::runtime_error);
	}
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

void TokenTest::AssertHasMore(Tokenizer &in) {
	CPPUNIT_ASSERT_MESSAGE("unexpected end of stream", in.HasMore());
}

void TokenTest::AssertToken(
	Token::Type expected_type,
	const Token &actual_token
) {
	AssertToken(expected_type, "", actual_token);
}

void TokenTest::AssertToken(
	Token::Type expected_type,
	string expected_value,
	const Token &actual_token
) {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected token type",
		expected_type, actual_token.type);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected token value",
		expected_value, actual_token.value);
}

}
}
