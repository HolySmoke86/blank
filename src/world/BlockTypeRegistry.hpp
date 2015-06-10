#ifndef BLANK_WORLD_BLOCKTYPEREGISTRY_HPP_
#define BLANK_WORLD_BLOCKTYPEREGISTRY_HPP_

#include "BlockType.hpp"

#include <vector>


namespace blank {

class BlockTypeRegistry {

public:
	BlockTypeRegistry();

public:
	Block::Type Add(const BlockType &);

	size_t Size() const noexcept { return types.size(); }

	BlockType &operator [](Block::Type id) { return types[id]; }
	const BlockType &Get(Block::Type id) const { return types[id]; }

private:
	std::vector<BlockType> types;

};

}

#endif
