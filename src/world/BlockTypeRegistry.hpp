#ifndef BLANK_WORLD_BLOCKTYPEREGISTRY_HPP_
#define BLANK_WORLD_BLOCKTYPEREGISTRY_HPP_

#include "BlockType.hpp"

#include <map>
#include <string>
#include <vector>


namespace blank {

class BlockTypeRegistry {

public:
	using size_type = std::vector<BlockType>::size_type;
	using reference = std::vector<BlockType>::reference;
	using const_reference = std::vector<BlockType>::const_reference;
	using iterator = std::vector<BlockType>::iterator;
	using const_iterator = std::vector<BlockType>::const_iterator;

public:
	BlockTypeRegistry();

	/// register a new block type
	/// this may throw if the name is already taken (names must be unique)
	Block::Type Add(BlockType &&);

	size_t size() const noexcept { return types.size(); }

	iterator begin() noexcept { return types.begin(); }
	const_iterator begin() const noexcept { return types.begin(); }
	iterator end() noexcept { return types.end(); }
	const_iterator end() const noexcept { return types.end(); }

	/// lookup by ID
	BlockType &operator [](Block::Type id) noexcept { return types[id]; }
	const BlockType &operator [](Block::Type id) const noexcept { return types[id]; }

	BlockType &Get(Block::Type id) noexcept { return types[id]; }
	const BlockType &Get(Block::Type id) const noexcept { return types[id]; }

	/// lookup by name
	BlockType &Get(const std::string &name);
	const BlockType &Get(const std::string &name) const;

private:
	std::vector<BlockType> types;
	std::map<std::string, Block::Type> names;

};

}

#endif
