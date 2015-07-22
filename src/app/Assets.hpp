#ifndef BLANK_APP_ASSETS_HPP_
#define BLANK_APP_ASSETS_HPP_

#include <string>


namespace blank {

class Font;

class Assets {

public:
	explicit Assets(const std::string &base);

	Font LoadFont(const std::string &name, int size) const;

private:
	std::string fonts;

};

}

#endif
