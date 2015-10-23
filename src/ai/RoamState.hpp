#ifndef BLANK_AI_ROAMSTATE_HPP_
#define BLANK_AI_ROAMSTATE_HPP_

#include "AIState.hpp"


namespace blank {

/// randomly waltz about the landscape
/// hold at random
/// start chasing a player if one comes near

class RoamState
: public AIState {

	void Enter(AIController &) const override;
	void Update(AIController &, Entity &, float dt) const override;
	void Exit(AIController &) const override;

};

}

#endif
