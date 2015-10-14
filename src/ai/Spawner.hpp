#ifndef BLANK_AI_SPAWNER_HPP_
#define BLANK_AI_SPAWNER_HPP_

#include "../app/IntervalTimer.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

class Controller;
class Entity;
class GaloisLFSR;
class Model;
class Skeletons;
class TextureIndex;
class World;

class Spawner {

public:
	Spawner(World &, Skeletons &, GaloisLFSR &);
	~Spawner();

	void LimitSkeletons(std::size_t begin, std::size_t end);
	void LoadTextures(TextureIndex &);

	void Update(int dt);

private:
	void CheckDespawn() noexcept;
	void TrySpawn();
	void Spawn(Entity &reference, const glm::ivec3 &, const glm::vec3 &);

	Model &RandomSkeleton() noexcept;

private:
	World &world;
	Skeletons &skeletons;
	std::vector<Controller *> controllers;

	GaloisLFSR &random;

	IntervalTimer timer;
	float despawn_range;
	float spawn_distance;
	unsigned int max_entities;
	int chunk_range;

	std::size_t skeletons_offset;
	std::size_t skeletons_length;

	std::vector<float> tex_map;

};

}

#endif
