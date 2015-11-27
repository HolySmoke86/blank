#ifndef BLANK_AI_IDLESTATE_HPP_
#define BLANK_AI_IDLESTATE_HPP_

#include "AIState.hpp"


namespace blank {

/// stand around and do nothing
/// occasionally look in a different direction
/// start roaming at random
/// start chasing a player if one comes near

class IdleState
: public AIState {

	void Enter(AIController &, Entity &) const override;
	void Update(AIController &, Entity &, float dt) const override;
	void Exit(AIController &, Entity &) const override;

};

}

#endif
