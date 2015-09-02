#ifndef BLANK_WORLD_CHUNKRENDERER_HPP_
#define BLANK_WORLD_CHUNKRENDERER_HPP_

#include "Block.hpp"
#include "Chunk.hpp"
#include "../graphics/ArrayTexture.hpp"
#include "../model/BlockModel.hpp"

#include <vector>


namespace blank {

class AssetLoader;
class TextureIndex;
class Viewport;
class World;

class ChunkRenderer {

public:
	/// render_distance in chunks, excluding the base chunk which is always rendered
	ChunkRenderer(World &, int render_distance);

	void LoadTextures(const AssetLoader &, const TextureIndex &);
	void FogDensity(float d) noexcept { fog_density = d; }

	bool InRange(const Chunk::Pos &) const noexcept;
	int IndexOf(const Chunk::Pos &) const noexcept;

	int TotalChunks() const noexcept { return total_length; }
	int IndexedChunks() const noexcept { return total_indexed; }
	int MissingChunks() const noexcept { return total_length - total_indexed; }

	void Rebase(const Chunk::Pos &);
	void Rescan();
	void Scan();
	void Update(int dt);

	void Render(Viewport &);

private:
	int GetCol(int) const noexcept;

	void Shift(Block::Face);

private:
	World &world;
	ArrayTexture block_tex;

	int render_dist;
	int side_length;
	int total_length;
	int total_indexed;
	glm::ivec3 stride;
	std::vector<BlockModel> models;
	std::vector<Chunk *> chunks;

	Chunk::Pos base;

	float fog_density;

};

}

#endif
