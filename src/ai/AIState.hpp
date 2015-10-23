#ifndef BLANK_AI_AISTATE_HPP_
#define BLANK_AI_AISTATE_HPP_


namespace blank {

struct AIState {

	virtual void Enter(AIController &) const = 0;
	virtual void Update(AIController &, Entity &, float dt) const = 0;
	virtual void Exit(AIController &) const = 0;

};

}

#endif
