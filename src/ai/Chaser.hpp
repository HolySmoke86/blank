#ifndef BLANK_AI_CHASER_HPP_
#define BLANK_AI_CHASER_HPP_

#include "Controller.hpp"


namespace blank {

class Chaser
: public Controller {

public:
	Chaser(Entity &ctrl, Entity &tgt) noexcept;
	~Chaser();

	Entity &Target() noexcept { return tgt; }
	const Entity &Target() const noexcept { return tgt; }

	void Update(int dt) override;

private:
	Entity &tgt;
	float speed;
	float stop_dist;
	float flee_dist;

};

}

#endif
