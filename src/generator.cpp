#include "generator.hpp"

#include <glm/glm.hpp>


namespace blank {

Generator::Generator(unsigned int seed)
: solidNoise(seed)
, typeNoise(seed + 1)
, stretch(64.0f)
, solid_threshold(0.8f)
, space(0)
, solids() {

}


void Generator::operator ()(Chunk &chunk) const {
	chunk.Allocate();
	Chunk::Pos pos(chunk.Position());
	glm::vec3 coords(pos * Chunk::Extent());
	for (int z = 0; z < Chunk::Depth(); ++z) {
		for (int y = 0; y < Chunk::Height(); ++y) {
			for (int x = 0; x < Chunk::Width(); ++x) {
				Block::Pos block_pos(x, y, z);
				glm::vec3 gen_pos = (coords + block_pos) / stretch;
				float val = solidNoise(gen_pos);
				if (val > solid_threshold) {
					int type_val = int((typeNoise(gen_pos) + 1.0f) * solids.size()) % solids.size();
					chunk.BlockAt(block_pos) = Block(solids[type_val]);
				} else {
					chunk.BlockAt(block_pos) = Block(space);
				}
			}
		}
	}
	chunk.Invalidate();
	chunk.CheckUpdate();
}

}
