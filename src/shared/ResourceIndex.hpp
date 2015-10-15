#ifndef BLANK_SHARED_RESOURCEINDEX_HPP_
#define BLANK_SHARED_RESOURCEINDEX_HPP_

#include <map>
#include <string>


namespace blank {

class ResourceIndex {

	using MapType = std::map<std::string, std::size_t>;

public:
	ResourceIndex();

	std::size_t GetID(const std::string &);

	std::size_t Size() const noexcept { return id_map.size(); }
	const MapType &Entries() const noexcept { return id_map; }

private:
	MapType id_map;

};

};

#endif
