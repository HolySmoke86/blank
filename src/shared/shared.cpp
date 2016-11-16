#include "ResourceIndex.hpp"
#include "WorldResources.hpp"

#include "../app/Assets.hpp"


namespace blank {

ResourceIndex::ResourceIndex()
: id_map() {

}

std::size_t ResourceIndex::GetID(const std::string &name) {
	auto entry = id_map.find(name);
	if (entry == id_map.end()) {
		auto result = id_map.emplace(name, Size());
		return result.first->second;
	} else {
		return entry->second;
	}
}


WorldResources::WorldResources()
: shapes()
, block_types()
, models()
, snd_index()
, tex_index() {

}

void WorldResources::Load(const AssetLoader &loader, const std::string &set_name) {
	loader.LoadShapes(set_name, shapes);
	loader.LoadBlockTypes(set_name, block_types, snd_index, tex_index, shapes);
	loader.LoadModels(set_name, models, tex_index, shapes);
}

}
