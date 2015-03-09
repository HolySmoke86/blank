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
, player()
, player_chunk(0, 0, 0)
, loaded()
, to_generate()
, to_free() {
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
	return
		a.Position().x * a.Position().x +
		a.Position().y * a.Position().y +
		a.Position().z * a.Position().z <
		b.Position().x * b.Position().x +
		b.Position().y * b.Position().y +
		b.Position().z * b.Position().z;
}

}

void World::Generate(const Chunk::Pos &from, const Chunk::Pos &to) {
	for (int z = from.z; z < to.z; ++z) {
		for (int y = from.y; y < to.y; ++y) {
			for (int x = from.x; x < to.x; ++x) {
				Block::Pos pos{float(x), float(y), float(z)};
				if (ChunkAvailable(pos)) {
					continue;
				} else if (x == 0 && y == 0 && z == 0) {
					loaded.emplace_back(blockType);
					loaded.back().Position(pos);
					Generate(loaded.back());
				} else {
					to_generate.emplace_back(blockType);
					to_generate.back().Position(pos);
				}
			}
		}
	}
	to_generate.sort(ChunkLess);
}

void World::Generate(Chunk &chunk) {
	chunk.Allocate();
	Chunk::Pos pos(chunk.Position());
	glm::vec3 coords(pos * Chunk::Extent());
	if (pos.x == 0 && pos.y == 0 && pos.z == 0) {
		for (size_t i = 1; i < blockType.Size(); ++i) {
			chunk.BlockAt(i) = Block(i);
			chunk.BlockAt(i + 257) = Block(i);
			chunk.BlockAt(i + 514) = Block(i);
		}
	} else {
		for (int z = 0; z < Chunk::Depth(); ++z) {
			for (int y = 0; y < Chunk::Height(); ++y) {
				for (int x = 0; x < Chunk::Width(); ++x) {
					Block::Pos block_pos{float(x), float(y), float(z)};
					glm::vec3 gen_pos = (coords + block_pos) / 64.0f;
					float val = blockNoise(gen_pos);
					if (val > 0.8f) {
						int col_val = int((colorNoise(gen_pos) + 1.0f) * 2.0f) % 4;
						chunk.BlockAt(block_pos) = Block(col_val * 3 + 1);
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
		if (cur_chunk.Intersection(ray, M * cur_chunk.Transform(player.ChunkCoords()), &cur_blkid, &cur_dist, &cur_normal)) {
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


Chunk *World::ChunkLoaded(const Chunk::Pos &pos) {
	for (Chunk &chunk : loaded) {
		if (chunk.Position() == pos) {
			return &chunk;
		}
	}
	return nullptr;
}

Chunk *World::ChunkQueued(const Chunk::Pos &pos) {
	for (Chunk &chunk : to_generate) {
		if (chunk.Position() == pos) {
			return &chunk;
		}
	}
	return nullptr;
}

Chunk *World::ChunkAvailable(const Chunk::Pos &pos) {
	Chunk *chunk = ChunkLoaded(pos);
	if (chunk) return chunk;

	return ChunkQueued(pos);
}

Chunk &World::Next(const Chunk &to, const glm::tvec3<int> &dir) {
	const Chunk::Pos tgt_pos = to.Position() + dir;

	Chunk *chunk = ChunkLoaded(tgt_pos);
	if (chunk) {
		return *chunk;
	}

	chunk = ChunkQueued(tgt_pos);
	if (chunk) {
		Generate(*chunk);
		return *chunk;
	}

	loaded.emplace_back(blockType);
	loaded.back().Position(tgt_pos);
	Generate(loaded.back());
	return loaded.back();
}


void World::Update(int dt) {
	player.Update(dt);

	CheckChunkGeneration();
}

void World::CheckChunkGeneration() {
	if (player.ChunkCoords() != player_chunk) {
		player_chunk = player.ChunkCoords();

		constexpr int max_dist = 8;
		// unload far away chunks
		for (auto iter(loaded.begin()), end(loaded.end()); iter != end;) {
			if (std::abs(player_chunk.x - iter->Position().x) > max_dist
					|| std::abs(player_chunk.y - iter->Position().y) > max_dist
					|| std::abs(player_chunk.z - iter->Position().z) > max_dist) {
				auto saved = iter;
				++iter;
				to_free.splice(to_free.end(), loaded, saved);
			} else {
				++iter;
			}
		}
		// abort far away queued chunks
		for (auto iter(to_generate.begin()), end(to_generate.end()); iter != end;) {
			if (std::abs(player_chunk.x - iter->Position().x) > max_dist
					|| std::abs(player_chunk.y - iter->Position().y) > max_dist
					|| std::abs(player_chunk.z - iter->Position().z) > max_dist) {
				iter = to_generate.erase(iter);
			} else {
				++iter;
			}
		}
		// add missing new chunks
		const Chunk::Pos offset(max_dist, max_dist, max_dist);
		Generate(player_chunk - offset, player_chunk + offset);
	}

	if (!to_generate.empty()) {
		Generate(to_generate.front());
		loaded.splice(loaded.end(), to_generate, to_generate.begin());
	}

	if (!to_free.empty()) {
		to_free.pop_front();
	}
}


void World::Render(DirectionalLighting &program) {
	program.SetLightDirection({ -1.0f, -3.0f, -2.0f });
	program.SetView(glm::inverse(player.Transform(player.ChunkCoords())));

	for (Chunk &chunk : LoadedChunks()) {
		glm::mat4 m(chunk.Transform(player.ChunkCoords()));
		program.SetM(m);
		glm::mat4 mvp(program.GetVP() * m);
		if (!CullTest(Chunk::Bounds(), mvp)) {
			chunk.Draw();
		}
	}
}

}
