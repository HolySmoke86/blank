#ifndef BLANK_AI_AICONTROLLER_HPP_
#define BLANK_AI_AICONTROLLER_HPP_

#include "../world/EntityController.hpp"

#include <glm/glm.hpp>


namespace blank {

class AIState;
class GaloisLFSR;

class AIController
: public EntityController {

public:
	explicit AIController(GaloisLFSR &);
	~AIController();

	void SetState(const AIState &);

	void Update(Entity &, float dt) override;

	glm::vec3 ControlForce(const EntityState &) const override;

	static glm::vec3 Heading(const EntityState &) noexcept;

	void StartFleeing(const Entity &, float speed) noexcept;
	void StopFleeing() noexcept;
	bool IsFleeing() const noexcept;
	const Entity &GetFleeTarget() const noexcept;

	void StartSeeking(const Entity &, float speed) noexcept;
	void StopSeeking() noexcept;
	bool IsSeeking() const noexcept;
	const Entity &GetSeekTarget() const noexcept;

	void StartWandering() noexcept;
	void StopWandering() noexcept;

private:
	GaloisLFSR &random;
	const AIState *state;

	const Entity *flee_target;
	float flee_speed;

	const Entity *seek_target;
	float seek_speed;

	bool wandering;
	glm::vec3 wander_pos;
	float wander_dist;
	float wander_radius;
	float wander_disp;
	float wander_speed;

};

}

#endif
