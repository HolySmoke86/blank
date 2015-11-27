#ifndef BLANK_AI_SPAWNER_HPP_
#define BLANK_AI_SPAWNER_HPP_

#include "../app/IntervalTimer.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

class Entity;
class Model;
class ModelRegistry;
class World;

class Spawner {

public:
	Spawner(World &, ModelRegistry &);
	~Spawner();

	void LimitModels(std::size_t begin, std::size_t end);

	void Update(int dt);

private:
	void CheckDespawn() noexcept;
	void TrySpawn();
	void Spawn(Entity &reference, const glm::ivec3 &, const glm::vec3 &);

	Model &RandomModel() noexcept;

private:
	World &world;
	ModelRegistry &models;
	std::vector<Entity *> entities;

	CoarseTimer timer;
	float despawn_range;
	float spawn_distance;
	unsigned int max_entities;
	int chunk_range;

	std::size_t model_offset;
	std::size_t model_length;

};

}

#endif
