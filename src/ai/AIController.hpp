#ifndef BLANK_AI_AICONTROLLER_HPP_
#define BLANK_AI_AICONTROLLER_HPP_

#include "../app/IntervalTimer.hpp"
#include "../geometry/primitive.hpp"
#include "../world/EntityController.hpp"

#include <glm/glm.hpp>


namespace blank {

class AIState;
class Entity;
class Player;
class World;

// TODO: AI and entities are tightly coupled, maybe AIcontroller should
//       be part of Entity. In that case, players could either be separated
//       from other entities use function as a degenerate AI which blindly
//       executes whatever its human tell it to.
class AIController
: public EntityController {

public:
	AIController(World &, Entity &);
	~AIController();

	void SetState(const AIState &, Entity &);

	void Update(Entity &, float dt) override;

	/// get the closest player that given entity can see
	/// returns nullptr if none are in sight
	Player *ClosestVisiblePlayer(const Entity &) noexcept;
	/// true if to entity is in visible range of from entity
	bool LineOfSight(const Entity &from, const Entity &to) const noexcept;

	/// true if the controller may do expensive calculations
	bool MayThink() const noexcept;
	void SetThinkInterval(float) noexcept;

	/// schedule a decision in the next minimum ± variance seconds
	void CueDecision(
		float minimum,
		float variance
	) noexcept;
	/// check if the scheduled decision is due already
	bool DecisionDue() const noexcept;
	/// random choice of 0 to num_choices - 1
	unsigned int Decide(unsigned int num_choices) noexcept;

private:
	World &world;
	const AIState *state;

	/// how far controlled entities can see
	float sight_dist;
	/// cosine of the half angle of FOV of controlled entities
	float sight_angle;

	FineTimer think_timer;
	FineTimer decision_timer;

};

}

#endif
