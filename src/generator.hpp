#ifndef BLANK_GENERATOR_HPP_
#define BLANK_GENERATOR_HPP_

#include "block.hpp"
#include "chunk.hpp"
#include "noise.hpp"

#include <vector>


namespace blank {

class Generator {

public:
	struct Config {
		unsigned int solid_seed = 0;
		unsigned int type_seed = 0;
		float stretch = 64.0f;
		float solid_threshold = 0.8f;
	};

	explicit Generator(const Config &);

	void operator ()(Chunk &) const;

	void Space(Block::Type t) { space = t; }
	void Light(Block::Type t) { light = t; }
	void Solids(const std::vector<Block::Type> &s) { solids = s; }

private:
	SimplexNoise solidNoise;
	WorleyNoise typeNoise;

	float stretch;
	float solid_threshold;

	Block::Type space;
	Block::Type light;
	std::vector<Block::Type> solids;

};

}

#endif
