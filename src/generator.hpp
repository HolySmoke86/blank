#ifndef BLANK_GENERATOR_HPP_
#define BLANK_GENERATOR_HPP_

#include "block.hpp"
#include "chunk.hpp"
#include "noise.hpp"

#include <vector>


namespace blank {

class Generator {

public:
	explicit Generator(unsigned int seed);

	void operator ()(Chunk &) const;

	void Space(Block::Type t) { space = t; }
	void Solids(const std::vector<Block::Type> &s) { solids = s; }

private:
	SimplexNoise solidNoise;
	SimplexNoise typeNoise;

	float stretch;
	float solid_threshold;

	Block::Type space;
	std::vector<Block::Type> solids;

};

}

#endif
