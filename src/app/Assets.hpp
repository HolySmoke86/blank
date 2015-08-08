#ifndef BLANK_APP_ASSETS_HPP_
#define BLANK_APP_ASSETS_HPP_

#include <string>


namespace blank {

class ArrayTexture;
class Font;
class Sound;
class Texture;

class Assets {

public:
	explicit Assets(const std::string &base);

	Font LoadFont(const std::string &name, int size) const;
	Sound LoadSound(const std::string &name) const;
	Texture LoadTexture(const std::string &name) const;
	void LoadTexture(const std::string &name, ArrayTexture &, int layer) const;

private:
	std::string fonts;
	std::string sounds;
	std::string textures;

};

}

#endif
