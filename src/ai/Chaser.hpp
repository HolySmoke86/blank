#ifndef BLANK_AI_CHASER_HPP_
#define BLANK_AI_CHASER_HPP_

#include "Controller.hpp"


namespace blank {

class World;

class Chaser
: public Controller {

public:
	Chaser(World &, Entity &ctrl, Entity &tgt) noexcept;
	~Chaser();

	Entity &Target() noexcept { return tgt; }
	const Entity &Target() const noexcept { return tgt; }

	void Update(int dt) override;

private:
	World &world;
	Entity &tgt;
	float chase_speed;
	float flee_speed;
	float stop_dist;
	float flee_dist;

};

}

#endif
