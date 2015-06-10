#ifndef BLANK_APP_RANDOMWALK_HPP_
#define BLANK_APP_RANDOMWALK_HPP_

#include <glm/glm.hpp>


namespace blank {

class Entity;

class RandomWalk {

public:
	explicit RandomWalk(Entity &) noexcept;

	void Update(int dt) noexcept;

private:
	Entity &entity;

	int time_left;

};

}

#endif
