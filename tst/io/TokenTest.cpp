#include "TokenTest.hpp"

#include "io/TokenStreamReader.hpp"

#include <sstream>
#include <stdexcept>
#include <glm/gtx/io.hpp>

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
	CPPUNIT_ASSERT_THROW_MESSAGE(
		"extracting token after EOS",
		in.Next(), std::runtime_error);
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


namespace {

template<class T>
void assert_read(std::string message, T expected, T actual, TokenStreamReader &in) {
	stringstream msg;
	msg << message << ", current token: " << in.Peek();
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		msg.str(),
		expected, actual);
}

}

void TokenTest::testReader() {
	stringstream ss;
	ss <<
		"/* booleans */\n"
		"true false yes no on off\n"
		"\"true\" \"false\" \"yes\" \"no\" \"on\" \"off\"\n"
		"1 0 -1\n"
		"# identifiers\n"
		"foo foo_bar vec.y\n"
		"// numbers\n"
		"0 1 +2 -3 4.5\n"
		".5 1.5 0.25 -1.75 0.625\n"
		"0 1 -1 2.5\n"
		// strings
		"\"hello\" \"\" \"\\r\\n\\t\\\"\"\n"
		// vectors
		"[1,0] [ 0.707, 0.707 ] // vec2\n"
		"[.577,.577 ,0.577] [ 1,-2,3] // vec3\n"
		"[ 0, 0, 0, 1 ] [1,0,0,-1.0] // vec4\n"
		"[640, 480] [3, 4, 5] [0, -10, 100, -1000] # ivecs\n"
		"[ -0.945, 0, -0.326, 0] # quat\n"
		;
	TokenStreamReader in(ss);

	// booleans

	bool value_bool;
	in.ReadBoolean(value_bool);
	assert_read("reading boolean true", true, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean false", false, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean yes", true, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean no", false, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean on", true, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean off", false, value_bool, in);

	in.ReadBoolean(value_bool);
	assert_read("reading boolean \"true\"", true, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean \"false\"", false, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean \"yes\"", true, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean \"no\"", false, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean \"on\"", true, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean \"off\"", false, value_bool, in);

	in.ReadBoolean(value_bool);
	assert_read("reading boolean 1", true, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean 0", false, value_bool, in);
	in.ReadBoolean(value_bool);
	assert_read("reading boolean -1", true, value_bool, in);

	// identifiers

	string value_ident;
	in.ReadIdentifier(value_ident);
	assert_read<string>("reading identifier foo", "foo", value_ident, in);
	in.ReadIdentifier(value_ident);
	assert_read<string>("reading identifier foo_bar", "foo_bar", value_ident, in);
	in.ReadIdentifier(value_ident);
	assert_read<string>("reading identifier vec.y", "vec.y", value_ident, in);

	// numbers
	int value_int;
	in.ReadNumber(value_int);
	assert_read("reading integer 0", 0, value_int, in);
	in.ReadNumber(value_int);
	assert_read("reading integer 1", 1, value_int, in);
	in.ReadNumber(value_int);
	assert_read("reading integer +2", 2, value_int, in);
	in.ReadNumber(value_int);
	assert_read("reading integer -3", -3, value_int, in);
	in.ReadNumber(value_int);
	assert_read("reading integer 4.5", 4, value_int, in);

	float value_float;
	in.ReadNumber(value_float);
	assert_read("reading float .5", .5f, value_float, in);
	in.ReadNumber(value_float);
	assert_read("reading float 1.5", 1.5f, value_float, in);
	in.ReadNumber(value_float);
	assert_read("reading float 0.25", .25f, value_float, in);
	in.ReadNumber(value_float);
	assert_read("reading float -1.75", -1.75f, value_float, in);
	in.ReadNumber(value_float);
	assert_read("reading float 0.625", 0.625f, value_float, in);

	unsigned long value_uint;
	in.ReadNumber(value_uint);
	assert_read("reading unsigned integer 0", 0ul, value_uint, in);
	in.ReadNumber(value_uint);
	assert_read("reading unsigned integer 1", 1ul, value_uint, in);
	in.ReadNumber(value_uint);
	assert_read("reading unsigned integer -1", -1ul, value_uint, in);
	in.ReadNumber(value_uint);
	assert_read("reading unsigned integer 2.5", 2ul, value_uint, in);

	// strings

	string value_string;
	in.ReadString(value_string);
	assert_read<string>(
		"reading string \"hello\"",
		"hello", value_string, in);
	in.ReadString(value_string);
	assert_read<string>(
		"reading string \"\"",
		"", value_string, in);
	in.ReadString(value_string);
	assert_read<string>(
		"reading string \"\\r\\n\\t\\\"\"",
		"\r\n\t\"", value_string, in);

	// vectors

	glm::vec2 value_vec2;
	in.ReadVec(value_vec2);
	assert_read(
		"reading vector [1,0]",
		glm::vec2(1, 0), value_vec2, in);
	in.ReadVec(value_vec2);
	assert_read(
		"reading vector [ 0.707, 0.707 ]",
		glm::vec2(.707, .707), value_vec2, in);

	glm::vec3 value_vec3;
	in.ReadVec(value_vec3);
	assert_read(
		"reading vector [.577,.577 ,0.577]",
		glm::vec3(.577, .577, .577), value_vec3, in);
	in.ReadVec(value_vec3);
	assert_read(
		"reading vector [ 1,-2,3]",
		glm::vec3(1, -2, 3), value_vec3, in);

	glm::vec4 value_vec4;
	in.ReadVec(value_vec4);
	assert_read(
		"reading vector [ 0, 0, 0, 1 ]",
		glm::vec4(0, 0, 0, 1), value_vec4, in);
	in.ReadVec(value_vec4);
	assert_read(
		"reading vector [1,0,0,-1.0]",
		glm::vec4(1, 0, 0, -1), value_vec4, in);

	glm::ivec2 value_ivec2;
	in.ReadVec(value_ivec2);
	assert_read(
		"reading integer vector [640, 480]",
		glm::ivec2(640, 480), value_ivec2, in);
	glm::ivec3 value_ivec3;
	in.ReadVec(value_ivec3);
	assert_read(
		"reading integer vector [3, 4, 5]",
		glm::ivec3(3, 4, 5), value_ivec3, in);
	glm::ivec4 value_ivec4;
	in.ReadVec(value_ivec4);
	assert_read(
		"reading integer vector [0, -10, 100, -1000]",
		glm::ivec4(0, -10, 100, -1000), value_ivec4, in);

	glm::quat value_quat;
	in.ReadQuat(value_quat);
	assert_read(
		"reading quaternion [ -0.945, 0, -0.326, 0]",
		glm::quat(-0.945, 0, -0.326, 0), value_quat, in);
}

void TokenTest::testReaderEmpty() {
	{ // zero length stream
		stringstream ss;
		ss << "";
		TokenStreamReader in(ss);
		CPPUNIT_ASSERT_MESSAGE(
			"empty stream shouldn't have tokens",
			!in.HasMore());
	}
	{ // stream consisting solely of comments
		stringstream ss;
		ss <<
			"/*\n"
			" * hello\n"
			" */\n"
			"#hello\n"
			"// is there anybody out there\n"
			;
		TokenStreamReader in(ss);
		CPPUNIT_ASSERT_MESSAGE(
			"comment stream shouldn't have tokens",
			!in.HasMore());
	}
}

void TokenTest::testReaderMalformed() {
	stringstream ss;
	ss << "";
	TokenStreamReader in(ss);
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
