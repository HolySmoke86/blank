#ifndef BLANK_IO_TOKENSTREAMREADER_HPP_
#define BLANK_IO_TOKENSTREAMREADER_HPP_

#include "Token.hpp"
#include "Tokenizer.hpp"

#include <iosfwd>
#include <string>
#include <glm/glm.hpp>


namespace blank {

class TokenStreamReader {

public:
	TokenStreamReader(std::istream &);

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

	void ReadVec(glm::vec2 &);
	void ReadVec(glm::vec3 &);
	void ReadVec(glm::vec4 &);

	void ReadVec(glm::ivec2 &);
	void ReadVec(glm::ivec3 &);
	void ReadVec(glm::ivec4 &);

	bool GetBool();
	float GetFloat();
	int GetInt();
	unsigned long GetULong();

private:
	void Assert(Token::Type);
	Token::Type GetType() const noexcept;
	const std::string &GetValue() const noexcept;

	Tokenizer in;
	bool cached;

};

}

#endif
