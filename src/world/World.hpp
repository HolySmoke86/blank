#ifndef BLANK_WORLD_WORLD_HPP_
#define BLANK_WORLD_WORLD_HPP_

#include "BlockTypeRegistry.hpp"
#include "ChunkLoader.hpp"
#include "Entity.hpp"
#include "Generator.hpp"
#include "../graphics/ArrayTexture.hpp"

#include <list>
#include <vector>
#include <glm/glm.hpp>


namespace blank {

class Assets;
class EntityCollision;
class Viewport;
class WorldCollision;

class World {

public:
	struct Config {
		// initial player position
		glm::vec3 spawn = { 0.0f, 0.0f, 0.0f };
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

	World(const Assets &, const Config &, const WorldSave &);

	/// check if this ray hits a block
	/// depth in the collision is the distance between the ray's
	/// origin and the intersection point
	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		WorldCollision &);

	/// check if this ray hits an entity
	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		EntityCollision &);

	/// check if given entity intersects with the world
	bool Intersection(const Entity &e, std::vector<WorldCollision> &);
	void Resolve(Entity &e, std::vector<WorldCollision> &);

	BlockTypeRegistry &BlockTypes() noexcept { return block_type; }
	ChunkLoader &Loader() noexcept { return chunks; }

	Entity &Player() { return *player; }
	Entity &AddEntity() { entities.emplace_back(); return entities.back(); }

	Chunk &PlayerChunk();
	Chunk &Next(const Chunk &to, const glm::ivec3 &dir);

	void Update(int dt);

	void Render(Viewport &);

private:
	BlockTypeRegistry block_type;

	ArrayTexture block_tex;

	Generator generate;
	ChunkLoader chunks;

	Entity *player;
	std::list<Entity> entities;

	glm::vec3 light_direction;
	float fog_density;

};

}

#endif
