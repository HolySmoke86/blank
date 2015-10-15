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
, tex_index() {

}

void WorldResources::Load(const AssetLoader &loader, const std::string &set) {
	loader.LoadShapes("default", shapes);
	loader.LoadBlockTypes("default", block_types, tex_index, shapes);
	loader.LoadModels("default", models, tex_index, shapes);
}

}
