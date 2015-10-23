#ifndef BLANK_AI_IDLESTATE_HPP_
#define BLANK_AI_IDLESTATE_HPP_

#include "AIState.hpp"


namespace blank {

class IdleState
: public AIState {

	void Enter(AIController &) const override;
	void Update(AIController &, Entity &, float dt) const override;
	void Exit(AIController &) const override;

};

}

#endif
