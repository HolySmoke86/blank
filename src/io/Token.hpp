#ifndef BLANK_IO_TOKEN_HPP_
#define BLANK_IO_TOKEN_HPP_

#include <string>


namespace blank {

struct Token {
	enum Type {
		UNKNOWN = 0,
		ANGLE_BRACKET_OPEN = '{',
		ANGLE_BRACKET_CLOSE = '}',
		CHEVRON_OPEN = '<',
		CHEVRON_CLOSE = '>',
		BRACKET_OPEN = '[',
		BRACKET_CLOSE = ']',
		PARENTHESIS_OPEN = '(',
		PARENTHESIS_CLOSE = ')',
		COLON = ':',
		SEMICOLON = ';',
		COMMA = ',',
		EQUALS = '=',
		NUMBER = '0',
		STRING = '"',
		IDENTIFIER = 'a',
		COMMENT = '#',
	} type = UNKNOWN;
	std::string value;
};

}

#endif
