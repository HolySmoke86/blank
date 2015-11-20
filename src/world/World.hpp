#ifndef BLANK_WORLD_WORLD_HPP_
#define BLANK_WORLD_WORLD_HPP_

#include "ChunkStore.hpp"
#include "Entity.hpp"
#include "Generator.hpp"
#include "Player.hpp"

#include <cstdint>
#include <list>
#include <string>
#include <vector>
#include <glm/glm.hpp>


namespace blank {

class BlockTypeRegistry;
class EntityCollision;
struct EntityDerivative;
class Viewport;
class WorldCollision;

class World {

public:
	struct Config {
		std::string name = "default";
		// chunk base where new players are spawned
		glm::ivec3 spawn = { 0, 0, 0 };
		// direction facing towards(!) the light
		glm::vec3 light_direction = { -1.0f, -3.0f, -2.0f };
		// fade out reaches 1/e (0.3679) at 1/fog_density,
		// gets less than 0.01 at e/(2 * fog_density)
		// I chose 0.011 because it yields 91 and 124 for those, so
		// slightly less than 6 and 8 chunks
		float fog_density = 0.011f;
	};

	World(const BlockTypeRegistry &, const Config &);
	~World();

	const std::string &Name() const noexcept { return config.name; }

	/// check if this ray hits a block
	/// depth in the collision is the distance between the ray's
	/// origin and the intersection point
	/// reference is the chunk offset of the ray in world space
	bool Intersection(
		const Ray &,
		const ExactLocation::Coarse &reference,
		WorldCollision &);

	/// check if this ray hits an entity
	/// intersections with the reference are not tested
	/// the ray is assumed to be in world space offset by entity's chunk coords
	bool Intersection(
		const Ray &,
		const Entity &reference,
		EntityCollision &);

	/// check if given entity intersects with the world
	bool Intersection(const Entity &e, const EntityState &, std::vector<WorldCollision> &);
	/// combine contacts into a single penetration vector
	/// depth is given to point towards position of given state
	static glm::vec3 CombinedInterpenetration(
		const EntityState &,
		const std::vector<WorldCollision> &) noexcept;

	/// check if given box (M * AABB) intersects with the world
	/// M is assumed to be calculated in reference to given chunk coords
	bool Intersection(
		const AABB &box,
		const glm::mat4 &M,
		const glm::ivec3 &reference,
		std::vector<WorldCollision> &);

	const BlockTypeRegistry &BlockTypes() noexcept { return block_type; }
	ChunkStore &Chunks() noexcept { return chunks; }

	/// add player with given name
	/// returns nullptr if the name is already taken
	Player *AddPlayer(const std::string &name);
	/// add player with given name and ID
	/// returns nullptr if the name or ID is already taken
	Player *AddPlayer(const std::string &name, std::uint32_t id);
	/// add an entity with an autogenerated ID
	Entity &AddEntity();
	/// add entity with given ID
	/// returns nullptr if the ID is already taken
	Entity *AddEntity(std::uint32_t id);
	/// add entity with given ID
	/// returs an existing entity if ID is already taken
	Entity &ForceAddEntity(std::uint32_t id);

	std::list<Player> &Players() noexcept { return players; }
	const std::list<Player> &Players() const noexcept { return players; }
	std::list<Entity> &Entities() noexcept { return entities; }
	const std::list<Entity> &Entities() const noexcept { return entities; }

	// dt in ms
	void Update(int dt);
	// dt in s
	void Update(Entity &, float dt);

	void Render(Viewport &);
	void RenderDebug(Viewport &);

private:
	using EntityHandle = std::list<Entity>::iterator;
	EntityHandle RemoveEntity(EntityHandle &);

	EntityDerivative CalculateStep(
		const Entity &,
		const EntityState &cur,
		float dt,
		const EntityDerivative &prev
	);
	glm::vec3 CalculateForce(
		const Entity &,
		const EntityState &cur
	);
	glm::vec3 ControlForce(
		const Entity &,
		const EntityState &
	);
	void CollisionFix(
		const Entity &,
		EntityState &
	);
	glm::vec3 Gravity(
		const Entity &,
		const EntityState &
	);

	/// calculate light direction and intensity at entity's location
	void GetLight(
		const Entity &entity,
		glm::vec3 &direction,
		glm::vec3 &color,
		glm::vec3 &ambient
	);

private:
	Config config;

	const BlockTypeRegistry &block_type;

	ChunkStore chunks;

	std::list<Player> players;
	std::list<Entity> entities;

	glm::vec3 light_direction;
	float fog_density;

};

}

#endif
