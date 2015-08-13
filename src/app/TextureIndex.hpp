#ifndef BLANK_APP_TEXTUREINDEX_HPP_
#define BLANK_APP_TEXTUREINDEX_HPP_

#include <map>
#include <string>


namespace blank {

class TextureIndex {

	using MapType = std::map<std::string, int>;

public:
	TextureIndex();

	int GetID(const std::string &);

	std::size_t Size() const noexcept { return id_map.size(); }
	const MapType &Entries() const noexcept { return id_map; }

private:
	MapType id_map;

};

};

#endif
