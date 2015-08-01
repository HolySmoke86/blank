#ifndef BLANK_APP_ASSETS_HPP_
#define BLANK_APP_ASSETS_HPP_

#include <string>


namespace blank {

class Font;
class Sound;

class Assets {

public:
	explicit Assets(const std::string &base);

	Font LoadFont(const std::string &name, int size) const;
	Sound LoadSound(const std::string &name) const;

private:
	std::string fonts;
	std::string sounds;

};

}

#endif
