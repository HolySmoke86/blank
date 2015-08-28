#ifndef BLANK_AI_RANDOMWALK_HPP_
#define BLANK_AI_RANDOMWALK_HPP_

#include "Controller.hpp"

#include "../rand/GaloisLFSR.hpp"


namespace blank {

/// Randomly start or stop moving in axis directions every now and then.
class RandomWalk
: public Controller {

public:
	RandomWalk(Entity &, std::uint64_t seed) noexcept;
	~RandomWalk();

	void Update(int dt) override;

private:
	GaloisLFSR random;
	int time_left;

};

}

#endif
