#ifndef BLANK_AI_SPAWNER_HPP_
#define BLANK_AI_SPAWNER_HPP_

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
	void Spawn(const glm::vec3 &);

private:
	World &world;
	std::list<RandomWalk> controllers;

};

}

#endif
