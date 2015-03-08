#ifndef BLANK_WORLD_HPP_
#define BLANK_WORLD_HPP_

#include "block.hpp"
#include "chunk.hpp"
#include "controller.hpp"
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

	FPSController &Controller() { return player; }

	Chunk &Next(const Chunk &, const glm::vec3 &dir);

	void Update(int dt);

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

	FPSController player;

	std::list<Chunk> loaded;
	std::list<Chunk> to_generate;

};

}

#endif
