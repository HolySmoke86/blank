#include "generator.hpp"

#include <glm/glm.hpp>


namespace blank {

Generator::Generator(const Config &config)
: solidNoise(config.solid_seed)
, typeNoise(config.type_seed)
, stretch(1.0f/config.stretch)
, solid_threshold(config.solid_threshold)
, space(0)
, light(0)
, solids() {

}


void Generator::operator ()(Chunk &chunk) const {
	Chunk::Pos pos(chunk.Position());
	glm::vec3 coords(pos * Chunk::Extent());
	for (int z = 0; z < Chunk::Depth(); ++z) {
		for (int y = 0; y < Chunk::Height(); ++y) {
			for (int x = 0; x < Chunk::Width(); ++x) {
				Block::Pos block_pos(x, y, z);
				glm::vec3 gen_pos = (coords + block_pos) * stretch;
				float val = OctaveNoise(solidNoise, coords + block_pos, 3, 0.5f, stretch, 2.0f);
				if (val > solid_threshold) {
					int type_val = int((typeNoise(gen_pos) + 1.0f) * solids.size()) % solids.size();
					chunk.SetBlock(block_pos, Block(solids[type_val]));
				} else {
					chunk.SetBlock(block_pos, Block(space));
				}
			}
		}
	}
	unsigned int random = 263167 * pos.x + 2097593 * pos.y + 426389 * pos.z;
	for (int index = 0; index < Chunk::Size(); ++index) {
		if (chunk.IsSurface(index)) {
			random = random * 666649 + 7778777;
			if ((random % 32) == 0) {
				chunk.SetBlock(index, Block(light));
			}
		}
	}
	chunk.Invalidate();
	chunk.CheckUpdate();
}

}
