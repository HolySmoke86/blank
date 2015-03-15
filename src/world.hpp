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
	World();

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

	Chunk &Next(const Chunk &to, const glm::tvec3<int> &dir);

	void Update(int dt);

	void Render(DirectionalLighting &);

private:
	BlockTypeRegistry blockType;
	CuboidShape blockShape;
	StairShape stairShape;
	CuboidShape slabShape;

	Generator generate;
	ChunkLoader chunks;

	Entity *player;
	std::list<Entity> entities;

};

}

#endif
