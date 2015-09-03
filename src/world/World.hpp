#ifndef BLANK_WORLD_WORLD_HPP_
#define BLANK_WORLD_WORLD_HPP_

#include "ChunkLoader.hpp"
#include "Entity.hpp"
#include "Generator.hpp"

#include <list>
#include <string>
#include <vector>
#include <glm/glm.hpp>


namespace blank {

class BlockTypeRegistry;
class EntityCollision;
class Viewport;
class WorldCollision;

class World {

public:
	struct Config {
		std::string name = "default";
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

	World(const BlockTypeRegistry &, const Config &, const WorldSave &);

	const std::string &Name() const noexcept { return config.name; }

	/// check if this ray hits a block
	/// depth in the collision is the distance between the ray's
	/// origin and the intersection point
	/// M is the global transform for given reference chunk
	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		const Chunk::Pos &reference,
		WorldCollision &);

	/// check if this ray hits an entity
	/// intersections with the reference are not tested
	/// M is the global transform for the chunk of given reference entity
	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		const Entity &reference,
		EntityCollision &);

	/// check if given entity intersects with the world
	bool Intersection(const Entity &e, std::vector<WorldCollision> &);
	void Resolve(Entity &e, std::vector<WorldCollision> &);

	const BlockTypeRegistry &BlockTypes() noexcept { return block_type; }
	ChunkLoader &Loader() noexcept { return chunks; }

	/// add player with given name
	/// returns nullptr if the name is already taken
	Entity *AddPlayer(const std::string &name);
	Entity &AddEntity() { entities.emplace_back(); return entities.back(); }

	const std::vector<Entity *> &Players() const noexcept { return players; }
	const std::list<Entity> &Entities() const noexcept { return entities; }

	void Update(int dt);

	void Render(Viewport &);

private:
	Config config;

	const BlockTypeRegistry &block_type;

	Generator generate;
	ChunkLoader chunks;

	std::vector<Entity *> players;
	std::list<Entity> entities;

	glm::vec3 light_direction;
	float fog_density;

};

}

#endif
