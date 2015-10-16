#ifndef BLANK_SHARED_WORLDRESOURCES_HPP_
#define BLANK_SHARED_WORLDRESOURCES_HPP_

#include "ResourceIndex.hpp"
#include "../model/ModelRegistry.hpp"
#include "../model/ShapeRegistry.hpp"
#include "../world/BlockTypeRegistry.hpp"

#include <string>


namespace blank {

class AssetLoader;

struct WorldResources {

	ShapeRegistry shapes;
	BlockTypeRegistry block_types;
	ModelRegistry models;

	ResourceIndex snd_index;
	ResourceIndex tex_index;


	WorldResources();

	void Load(const AssetLoader &, const std::string &set);

};

}

#endif
