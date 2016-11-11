#ifndef BLANK_APP_ASSETS_HPP_
#define BLANK_APP_ASSETS_HPP_

#include "../graphics/Font.hpp"

#include <string>


namespace blank {

class ArrayTexture;
class BlockTypeRegistry;
class CubeMap;
class ModelRegistry;
class ResourceIndex;
class ShapeRegistry;
class Sound;
class Texture;

class AssetLoader {

public:
	explicit AssetLoader(const std::string &base);

	void LoadBlockTypes(
		const std::string &set_name,
		BlockTypeRegistry &,
		ResourceIndex &snd,
		ResourceIndex &tex,
		const ShapeRegistry &) const;
	CubeMap LoadCubeMap(const std::string &name) const;
	Font LoadFont(const std::string &name, int size) const;
	void LoadModels(
		const std::string &set_name,
		ModelRegistry &,
		ResourceIndex &,
		const ShapeRegistry &) const;
	void LoadShapes(const std::string &set_name, ShapeRegistry &) const;
	Sound LoadSound(const std::string &name) const;
	Texture LoadTexture(const std::string &name) const;
	void LoadTexture(const std::string &name, ArrayTexture &, int layer) const;
	void LoadTextures(const ResourceIndex &, ArrayTexture &) const;

private:
	std::string fonts;
	std::string sounds;
	std::string textures;
	std::string data;

};

struct Assets {

	Font large_ui_font;
	Font small_ui_font;

	explicit Assets(const AssetLoader &);

};

}

#endif
