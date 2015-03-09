#ifndef BLANK_WORLD_HPP_
#define BLANK_WORLD_HPP_

#include "block.hpp"
#include "chunk.hpp"
#include "entity.hpp"
#include "noise.hpp"
#include "shader.hpp"
#include "shape.hpp"

#include <list>
#include <glm/glm.hpp>


namespace blank {

class World {

public:
	World();

	void Generate(const Chunk::Pos &from, const Chunk::Pos &to);

	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		Chunk **chunk = nullptr,
		int *blkid = nullptr,
		float *dist = nullptr,
		glm::vec3 *normal = nullptr);

	BlockTypeRegistry &BlockTypes() { return blockType; }
	std::list<Chunk> &LoadedChunks() { return loaded; }

	Entity &Player() { return player; }

	Chunk *ChunkLoaded(const Chunk::Pos &);
	Chunk *ChunkQueued(const Chunk::Pos &);
	Chunk *ChunkAvailable(const Chunk::Pos &);
	Chunk &Next(const Chunk &, const glm::tvec3<int> &dir);

	void Update(int dt);
	void CheckChunkGeneration();

	void Render(DirectionalLighting &);

private:
	void Generate(Chunk &);

private:
	BlockTypeRegistry blockType;
	CuboidShape blockShape;
	StairShape stairShape;
	CuboidShape slabShape;

	SimplexNoise blockNoise;
	SimplexNoise colorNoise;

	Entity player;
	Chunk::Pos player_chunk;

	std::list<Chunk> loaded;
	std::list<Chunk> to_generate;
	std::list<Chunk> to_free;

};

}

#endif
