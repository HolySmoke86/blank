#ifndef BLANK_WORLD_HPP_
#define BLANK_WORLD_HPP_

#include "block.hpp"
#include "chunk.hpp"
#include "entity.hpp"
#include "generator.hpp"
#include "shader.hpp"
#include "shape.hpp"

#include <list>
#include <glm/glm.hpp>


namespace blank {

class World {

public:
	struct Config {
		// initial player position
		glm::vec3 spawn = { 4.0f, 4.0f, 4.0f };
		// direction facing towards(!) the light
		glm::vec3 light_direction = { -1.0f, -3.0f, -2.0f };
		// fade out reaches 1/e (0.3679) at 1/fog_density,
		// gets less than 0.01 at e/(2 * fog_density)
		// I chose 0.011 because it yields 91 and 124 for those, so
		// slightly less than 6 and 8 chunks
		float fog_density = 0.011f;

		Generator::Config gen = Generator::Config();
		ChunkLoader::Config load = ChunkLoader::Config();
	};

	explicit World(const Config &);

	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		Chunk **chunk = nullptr,
		int *blkid = nullptr,
		float *dist = nullptr,
		glm::vec3 *normal = nullptr);

	BlockTypeRegistry &BlockTypes() { return blockType; }

	Entity &Player() { return *player; }
	Entity &AddEntity() { entities.emplace_back(); return entities.back(); }

	Chunk &PlayerChunk();
	Chunk &Next(const Chunk &to, const glm::tvec3<int> &dir);

	void Update(int dt);

	void Render(BlockLighting &, DirectionalLighting &);

private:
	BlockTypeRegistry blockType;
	CuboidShape blockShape;
	StairShape stairShape;
	CuboidShape slabShape;

	Generator generate;
	ChunkLoader chunks;

	Entity *player;
	std::list<Entity> entities;

	glm::vec3 light_direction;
	float fog_density;

};

}

#endif
