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

	void Generate(const glm::tvec3<int> &from, const glm::tvec3<int> &to);

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

	Chunk *ChunkLoaded(const glm::tvec3<int> &);
	Chunk *ChunkQueued(const glm::tvec3<int> &);
	Chunk *ChunkAvailable(const glm::tvec3<int> &);
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
	glm::tvec3<int> player_chunk;

	std::list<Chunk> loaded;
	std::list<Chunk> to_generate;
	std::list<Chunk> to_free;

};

}

#endif
