#include "Tokenizer.hpp"
#include "TokenStreamReader.hpp"

#include <cctype>
#include <istream>
#include <stdexcept>

using namespace std;


namespace blank {

Tokenizer::Tokenizer(istream &in)
: in(in)
, current() {

}


bool Tokenizer::HasMore() {
	return bool(istream::sentry(in));
}

const Token &Tokenizer::Next() {
	ReadToken();
	return Current();
}

void Tokenizer::ReadToken() {
	current.type = Token::UNKNOWN;
	current.value.clear();

	istream::sentry s(in);
	if (!s) {
		// TODO: error?
		return;
	}

	istream::char_type c;
	in.get(c);
	switch (c) {
		case '{': case '}':
		case '<': case '>':
		case '[': case ']':
		case '(': case ')':
		case ';': case ':':
		case ',': case '=':
			current.type = Token::Type(c);
			break;
		case '+': case '-':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			in.putback(c);
			ReadNumber();
			break;
		case '"':
			ReadString();
			break;
		case '#':
		case '/':
			in.putback(c);
			ReadComment();
			break;
		default:
			in.putback(c);
			ReadIdentifier();
			break;
	}
}

namespace {

bool is_num_char(istream::char_type c) {
	return isdigit(c)
		|| c == '.'
		|| c == '-'
		|| c == '+'
	;
}

}

void Tokenizer::ReadNumber() {
	current.type = Token::NUMBER;
	istream::char_type c;
	while (in.get(c)) {
		if (is_num_char(c)) {
			current.value += c;
		} else {
			in.putback(c);
			break;
		}
	}
}

void Tokenizer::ReadString() {
	current.type = Token::STRING;
	bool escape = false;

	istream::char_type c;
	while (in.get(c)) {
		if (escape) {
			escape = false;
			switch (c) {
				case 'n':
					current.value += '\n';
					break;
				case 'r':
					current.value += '\t';
					break;
				case 't':
					current.value += '\t';
					break;
				default:
					current.value += c;
					break;
			}
		} else if (c == '"') {
			break;
		} else if (c == '\\') {
			escape = true;
		} else {
			current.value += c;
		}
	}
}

void Tokenizer::ReadComment() {
	current.type = Token::COMMENT;
	istream::char_type c;
	in.get(c);

	if (c == '#') {
		while (in.get(c) && c != '\n') {
			current.value += c;
		}
		return;
	}

	// c is guaranteed to be '/' now
	if (!in.get(c)) {
		throw runtime_error("unexpected end of stream");
	}
	if (c == '/') {
		while (in.get(c) && c != '\n') {
			current.value += c;
		}
		return;
	} else if (c != '*') {
		throw runtime_error("invalid character after /");
	}

	while (in.get(c)) {
		if (c == '*') {
			istream::char_type c2;
			if (!in.get(c2)) {
				throw runtime_error("unexpected end of stream");
			}
			if (c2 == '/') {
				break;
			} else {
				current.value += c;
				current.value += c2;
			}
		} else {
			current.value += c;
		}
	}
}

void Tokenizer::ReadIdentifier() {
	current.type = Token::IDENTIFIER;

	istream::char_type c;
	while (in.get(c)) {
		if (isalnum(c) || c == '_') {
			current.value += c;
		} else {
			in.putback(c);
			break;
		}
	}
}


TokenStreamReader::TokenStreamReader(istream &in)
: in(in)
, cached(false) {

}


bool TokenStreamReader::HasMore() {
	while (in.HasMore()) {
		if (in.Next().type != Token::COMMENT) {
			cached = true;
			return true;
		}
	}
	return false;
}

const Token &TokenStreamReader::Next() {
	if (cached) {
		cached = false;
		return in.Current();
	} else {
		return in.Next();
	}
}

const Token &TokenStreamReader::Peek() {
	if (!cached) {
		in.Next();
		cached = true;
	}
	return in.Current();
}


void TokenStreamReader::Assert(Token::Type t) {
	if (GetType() != t) {
		throw runtime_error("unexpected token in input stream");
	}
}

Token::Type TokenStreamReader::GetType() const noexcept {
	return in.Current().type;
}

const std::string &TokenStreamReader::GetValue() const noexcept {
	return in.Current().value;
}

void TokenStreamReader::Skip(Token::Type t) {
	Next();
	Assert(t);
}


void TokenStreamReader::ReadBoolean(bool &b) {
	b = GetBool();
}

void TokenStreamReader::ReadIdentifier(string &out) {
	Next();
	Assert(Token::IDENTIFIER);
	out = GetValue();
}

void TokenStreamReader::ReadNumber(float &n) {
	n = GetFloat();
}

void TokenStreamReader::ReadNumber(int &n) {
	n = GetInt();
}

void TokenStreamReader::ReadNumber(unsigned long &n) {
	n = GetULong();
}

void TokenStreamReader::ReadString(string &out) {
	Next();
	Assert(Token::STRING);
	out = GetValue();
}


void TokenStreamReader::ReadVec(glm::vec2 &v) {
	Skip(Token::BRACKET_OPEN);
	ReadNumber(v.x);
	Skip(Token::COMMA);
	ReadNumber(v.y);
	Skip(Token::BRACKET_CLOSE);
}

void TokenStreamReader::ReadVec(glm::vec3 &v) {
	Skip(Token::BRACKET_OPEN);
	ReadNumber(v.x);
	Skip(Token::COMMA);
	ReadNumber(v.y);
	Skip(Token::COMMA);
	ReadNumber(v.z);
	Skip(Token::BRACKET_CLOSE);
}

void TokenStreamReader::ReadVec(glm::vec4 &v) {
	Skip(Token::BRACKET_OPEN);
	ReadNumber(v.x);
	Skip(Token::COMMA);
	ReadNumber(v.y);
	Skip(Token::COMMA);
	ReadNumber(v.z);
	Skip(Token::COMMA);
	ReadNumber(v.w);
	Skip(Token::BRACKET_CLOSE);
}

void TokenStreamReader::ReadVec(glm::ivec2 &v) {
	Skip(Token::BRACKET_OPEN);
	ReadNumber(v.x);
	Skip(Token::COMMA);
	ReadNumber(v.y);
	Skip(Token::BRACKET_CLOSE);
}

void TokenStreamReader::ReadVec(glm::ivec3 &v) {
	Skip(Token::BRACKET_OPEN);
	ReadNumber(v.x);
	Skip(Token::COMMA);
	ReadNumber(v.y);
	Skip(Token::COMMA);
	ReadNumber(v.z);
	Skip(Token::BRACKET_CLOSE);
}

void TokenStreamReader::ReadVec(glm::ivec4 &v) {
	Skip(Token::BRACKET_OPEN);
	ReadNumber(v.x);
	Skip(Token::COMMA);
	ReadNumber(v.y);
	Skip(Token::COMMA);
	ReadNumber(v.z);
	Skip(Token::COMMA);
	ReadNumber(v.w);
	Skip(Token::BRACKET_CLOSE);
}


bool TokenStreamReader::GetBool() {
	Next();
	switch (GetType()) {
		case Token::NUMBER:
			return GetInt() != 0;
		case Token::IDENTIFIER:
		case Token::STRING:
			if (GetValue() == "true" || GetValue() == "yes" || GetValue() == "on") {
				return true;
			} else if (GetValue() == "false" || GetValue() == "no" || GetValue() == "off") {
				return false;
			} else {
				throw runtime_error("unexpected value in input stream");
			}
		default:
			throw runtime_error("unexpected token in input stream");
	}
}

float TokenStreamReader::GetFloat() {
	Next();
	Assert(Token::NUMBER);
	return stof(GetValue());
}

int TokenStreamReader::GetInt() {
	Next();
	Assert(Token::NUMBER);
	return stoi(GetValue());
}

unsigned long TokenStreamReader::GetULong() {
	Next();
	Assert(Token::NUMBER);
	return stoul(GetValue());
}

}
