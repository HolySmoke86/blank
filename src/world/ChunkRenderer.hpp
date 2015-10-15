#ifndef BLANK_WORLD_CHUNKRENDERER_HPP_
#define BLANK_WORLD_CHUNKRENDERER_HPP_

#include "Block.hpp"
#include "Chunk.hpp"
#include "../graphics/ArrayTexture.hpp"
#include "../graphics/BlockMesh.hpp"

#include <vector>


namespace blank {

class AssetLoader;
class BlockMesh;
class ChunkIndex;
class ResourceIndex;
class Viewport;

class ChunkRenderer {

public:
	explicit ChunkRenderer(ChunkIndex &);
	~ChunkRenderer();

	void LoadTextures(const AssetLoader &, const ResourceIndex &);
	void FogDensity(float d) noexcept { fog_density = d; }

	int MissingChunks() const noexcept;

	void Update(int dt);

	void Render(Viewport &);

private:
	ChunkIndex &index;
	std::vector<BlockMesh> models;

	ArrayTexture block_tex;

	float fog_density;

};

}

#endif
