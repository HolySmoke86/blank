#ifndef BLANK_AI_SPAWNER_HPP_
#define BLANK_AI_SPAWNER_HPP_

#include "../app/IntervalTimer.hpp"

#include <list>
#include <glm/glm.hpp>


namespace blank {

class RandomWalk;
class World;

class Spawner {

public:
	explicit Spawner(World &);
	~Spawner();

	void Update(int dt);

private:
	void CheckDespawn() noexcept;
	void TrySpawn();
	void Spawn(const glm::tvec3<int> &, const glm::vec3 &);

private:
	World &world;
	std::list<RandomWalk> controllers;

	IntervalTimer timer;
	float despawn_range;
	float spawn_distance;
	unsigned int max_entities;
	int chunk_range;

};

}

#endif
