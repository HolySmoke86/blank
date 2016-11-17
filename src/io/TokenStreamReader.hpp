#ifndef BLANK_IO_TOKENSTREAMREADER_HPP_
#define BLANK_IO_TOKENSTREAMREADER_HPP_

#include "Token.hpp"
#include "Tokenizer.hpp"
#include "../graphics/glm.hpp"

#include <iosfwd>
#include <string>


namespace blank {

class TokenStreamReader {

public:
	explicit TokenStreamReader(std::istream &);

	bool HasMore();
	const Token &Next();
	const Token &Peek();

	void Skip(Token::Type);

	void ReadBoolean(bool &);
	void ReadIdentifier(std::string &);
	void ReadNumber(float &);
	void ReadNumber(int &);
	void ReadNumber(unsigned long &);
	void ReadString(std::string &);
	// like ReadString, but does not require the value to be
	// written as a string literal in source
	void ReadRelaxedString(std::string &);

	void ReadVec(glm::vec2 &);
	void ReadVec(glm::vec3 &);
	void ReadVec(glm::vec4 &);

	void ReadVec(glm::ivec2 &);
	void ReadVec(glm::ivec3 &);
	void ReadVec(glm::ivec4 &);

	void ReadQuat(glm::quat &);

	// the Get* functions advance to the next token
	// the As* functions try to cast the current token
	// if the value could not be converted, a std::runtime_error is thrown
	// conversion to string is always possible

	bool GetBool();
	bool AsBool() const;
	float GetFloat();
	float AsFloat() const;
	int GetInt();
	int AsInt() const;
	unsigned long GetULong();
	unsigned long AsULong() const;
	const std::string &GetString();
	const std::string &AsString() const;

private:
	void SkipComments();

	void Assert(Token::Type) const;
	Token::Type GetType() const noexcept;
	const std::string &GetValue() const noexcept;

	Tokenizer in;
	bool cached;

};

}

#endif
