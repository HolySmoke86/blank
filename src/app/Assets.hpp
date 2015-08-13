#ifndef BLANK_APP_ASSETS_HPP_
#define BLANK_APP_ASSETS_HPP_

#include "../graphics/Font.hpp"

#include <string>


namespace blank {

class ArrayTexture;
class BlockTypeRegistry;
class Sound;
class Texture;
class TextureIndex;

class Assets {

public:
	explicit Assets(const std::string &base);

	void LoadBlockTypes(const std::string &set_name, BlockTypeRegistry &, TextureIndex &) const;
	Font LoadFont(const std::string &name, int size) const;
	Sound LoadSound(const std::string &name) const;
	Texture LoadTexture(const std::string &name) const;
	void LoadTexture(const std::string &name, ArrayTexture &, int layer) const;
	void LoadTextures(const TextureIndex &, ArrayTexture &) const;

private:
	std::string fonts;
	std::string sounds;
	std::string textures;
	std::string data;

public:
	// common assets shared by may states
	Font large_ui_font;
	Font small_ui_font;

};

}

#endif
