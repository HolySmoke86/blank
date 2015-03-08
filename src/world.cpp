#include "world.hpp"

#include <limits>
#include <glm/gtx/transform.hpp>


namespace blank {

World::World()
: blockType()
, blockShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }})
, stairShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }}, { 0.0f, 0.0f })
, slabShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.0f, 0.5f }})
, blockNoise(0)
, colorNoise(1)
, loaded()
, to_generate() {
	blockType.Add(BlockType{ true, { 1.0f, 1.0f, 1.0f }, &blockShape }); // white block
	blockType.Add(BlockType{ true, { 1.0f, 1.0f, 1.0f }, &stairShape }); // white stair
	blockType.Add(BlockType{ true, { 1.0f, 1.0f, 1.0f }, &slabShape }); // white slab
	blockType.Add(BlockType{ true, { 1.0f, 0.0f, 0.0f }, &blockShape }); // red block
	blockType.Add(BlockType{ true, { 1.0f, 0.0f, 0.0f }, &stairShape }); // red stair
	blockType.Add(BlockType{ true, { 1.0f, 0.0f, 0.0f }, &slabShape }); // red slab
	blockType.Add(BlockType{ true, { 0.0f, 1.0f, 0.0f }, &blockShape }); // green block
	blockType.Add(BlockType{ true, { 0.0f, 1.0f, 0.0f }, &stairShape }); // green stair
	blockType.Add(BlockType{ true, { 0.0f, 1.0f, 0.0f }, &slabShape }); // green slab
	blockType.Add(BlockType{ true, { 0.0f, 0.0f, 1.0f }, &blockShape }); // blue block
	blockType.Add(BlockType{ true, { 0.0f, 0.0f, 1.0f }, &stairShape }); // blue stair
	blockType.Add(BlockType{ true, { 0.0f, 0.0f, 1.0f }, &slabShape }); // blue slab

	player.Position({ 4.0f, 4.0f, 4.0f });
}


namespace {

bool ChunkLess(const Chunk &a, const Chunk &b) {
	return dot(a.Position(), a.Position()) < dot(b.Position(), b.Position());
}

}

void World::Generate(const glm::tvec3<int> &from, const glm::tvec3<int> &to) {
	for (int z = from.z; z < to.z; ++z) {
		for (int y = from.y; y < to.y; ++y) {
			for (int x = from.x; x < to.x; ++x) {
				glm::vec3 pos{float(x), float(y), float(z)};
				if (x == 0 && y == 0 && z == 0) {
					loaded.emplace_back();
					loaded.back().Position(pos);
					Generate(loaded.back());
				} else {
					to_generate.emplace_back();
					to_generate.back().Position(pos);
				}
			}
		}
	}
	to_generate.sort(ChunkLess);
}

void World::Generate(Chunk &chunk) {
	glm::vec3 pos(chunk.Position());
	if (pos.x == 0 && pos.y == 0 && pos.z == 0) {
		for (size_t i = 1; i < blockType.Size(); ++i) {
			chunk.BlockAt(i) = Block(blockType[i]);
			chunk.BlockAt(i + 257) = Block(blockType[i]);
			chunk.BlockAt(i + 514) = Block(blockType[i]);
		}
	} else {
		for (int z = 0; z < Chunk::Depth(); ++z) {
			for (int y = 0; y < Chunk::Height(); ++y) {
				for (int x = 0; x < Chunk::Width(); ++x) {
					glm::vec3 block_pos{float(x), float(y), float(z)};
					glm::vec3 gen_pos = (pos * Chunk::Extent() + block_pos) / 64.0f;
					float val = blockNoise(gen_pos);
					if (val > 0.8f) {
						int col_val = int((colorNoise(gen_pos) + 1.0f) * 2.0f) % 4;
						chunk.BlockAt(block_pos) = Block(blockType[col_val * 3 + 1]);
					}
				}
			}
		}
	}
	chunk.Invalidate();
}

bool World::Intersection(
		const Ray &ray,
		const glm::mat4 &M,
		Chunk **chunk,
		int *blkid,
		float *dist,
		glm::vec3 *normal) {
	Chunk *closest_chunk = nullptr;
	int closest_blkid = -1;
	float closest_dist = std::numeric_limits<float>::infinity();
	glm::vec3 closest_normal;

	for (Chunk &cur_chunk : loaded) {
		int cur_blkid;
		float cur_dist;
		glm::vec3 cur_normal;
		if (cur_chunk.Intersection(ray, M * cur_chunk.Transform(), &cur_blkid, &cur_dist, &cur_normal)) {
			if (cur_dist < closest_dist) {
				closest_chunk = &cur_chunk;
				closest_blkid = cur_blkid;
				closest_dist = cur_dist;
				closest_normal = cur_normal;
			}
		}
	}

	if (chunk) {
		*chunk = closest_chunk;
	}
	if (blkid) {
		*blkid = closest_blkid;
	}
	if (dist) {
		*dist = closest_dist;
	}
	if (normal) {
		*normal = closest_normal;
	}
	return closest_chunk;
}


Chunk &World::Next(const Chunk &to, const glm::vec3 &dir) {
	const glm::vec3 tgt_pos = to.Position() + dir;
	for (Chunk &chunk : LoadedChunks()) {
		if (chunk.Position() == tgt_pos) {
			return chunk;
		}
	}
	for (Chunk &chunk : to_generate) {
		if (chunk.Position() == tgt_pos) {
			Generate(chunk);
			return chunk;
		}
	}
	loaded.emplace_back();
	loaded.back().Position(tgt_pos);
	Generate(loaded.back());
	return loaded.back();
}


void World::Update(int dt) {
	player.Update(dt);

	if (!to_generate.empty()) {
		Generate(to_generate.front());
		loaded.splice(loaded.end(), to_generate, to_generate.begin());
	}
}


void World::Render(DirectionalLighting &program) {
	program.SetLightDirection({ -1.0f, -3.0f, -2.0f });
	program.SetView(glm::inverse(player.Transform()));

	for (Chunk &chunk : LoadedChunks()) {
		program.SetM(chunk.Transform());
		chunk.Draw();
	}
}

}
