#ifndef BLANK_WORLD_CHUNKRENDERER_HPP_
#define BLANK_WORLD_CHUNKRENDERER_HPP_

#include "Block.hpp"
#include "Chunk.hpp"
#include "../graphics/ArrayTexture.hpp"
#include "../model/BlockModel.hpp"

#include <vector>


namespace blank {

class AssetLoader;
class BlockModel;
class ChunkIndex;
class TextureIndex;
class Viewport;

class ChunkRenderer {

public:
	explicit ChunkRenderer(ChunkIndex &);
	~ChunkRenderer();

	void LoadTextures(const AssetLoader &, const TextureIndex &);
	void FogDensity(float d) noexcept { fog_density = d; }

	int MissingChunks() const noexcept;

	void Update(int dt);

	void Render(Viewport &);

private:
	ChunkIndex &index;
	std::vector<BlockModel> models;

	ArrayTexture block_tex;

	float fog_density;

};

}

#endif
