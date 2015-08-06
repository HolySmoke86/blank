#ifndef BLANK_AI_RANDOMWALK_HPP_
#define BLANK_AI_RANDOMWALK_HPP_

#include "Controller.hpp"


namespace blank {

/// Randomly start or stop moving in axis directions every now and then.
class RandomWalk
: public Controller {

public:
	explicit RandomWalk(Entity &) noexcept;
	~RandomWalk();

	void Update(int dt) override;

private:
	int time_left;

};

}

#endif
