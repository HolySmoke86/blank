#ifndef BLANK_AI_RANDOMWALK_HPP_
#define BLANK_AI_RANDOMWALK_HPP_

#include "Controller.hpp"

#include "../rand/GaloisLFSR.hpp"

#include <glm/glm.hpp>


namespace blank {

/// Randomly start or stop moving in axis directions every now and then.
class RandomWalk
: public Controller {

public:
	RandomWalk(Entity &, std::uint64_t seed) noexcept;
	~RandomWalk();

	void Update(int dt) override;

private:
	void Change() noexcept;

private:
	GaloisLFSR random;

	glm::vec3 start_vel;
	glm::vec3 target_vel;

	int switch_time;
	float lerp_max;
	float lerp_time;

};

}

#endif
