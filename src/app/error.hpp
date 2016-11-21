#ifndef BLANK_APP_ERROR_HPP_
#define BLANK_APP_ERROR_HPP_

#include <al.h>
#include <stdexcept>
#include <string>


namespace blank {

class AlutError
: public std::runtime_error {

public:
	explicit AlutError(ALenum);
	AlutError(ALenum, const std::string &);

};


class GLError
: public std::runtime_error {

public:
	GLError();
	explicit GLError(const std::string &);

};


class NetError
: public std::runtime_error {

public:
	NetError();
	explicit NetError(const std::string &);

};


class SDLError
: public std::runtime_error {

public:
	SDLError();
	explicit SDLError(const std::string &);

};


class SysError
: public std::runtime_error {

public:
	SysError();
	explicit SysError(const std::string &);
	explicit SysError(int err_num);
	SysError(int err_num, const std::string &);

};


class TTFError
: public std::runtime_error {

public:
	TTFError();
	explicit TTFError(const std::string &);

};

}

#endif
