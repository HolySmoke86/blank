#ifndef BLANK_AI_SPAWNER_HPP_
#define BLANK_AI_SPAWNER_HPP_

#include "../app/IntervalTimer.hpp"
#include "../model/EntityModel.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

class Controller;
class World;

class Spawner {

public:
	explicit Spawner(World &);
	~Spawner();

	void Update(int dt);

private:
	void CheckDespawn() noexcept;
	void TrySpawn();
	void Spawn(const glm::ivec3 &, const glm::vec3 &);

private:
	World &world;
	std::vector<Controller *> controllers;

	EntityModel models[3];

	IntervalTimer timer;
	float despawn_range;
	float spawn_distance;
	unsigned int max_entities;
	int chunk_range;

};

}

#endif
