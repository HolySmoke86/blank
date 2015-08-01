#ifndef BLANK_AUDIO_ALERROR_HPP_
#define BLANK_AUDIO_ALERROR_HPP_

#include <al.h>
#include <stdexcept>
#include <string>


namespace blank {

class ALError
: public std::runtime_error {

public:
	explicit ALError(ALenum);
	ALError(ALenum, const std::string &);

};

}

#endif
