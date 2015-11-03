#ifndef BLANK_AI_AICONTROLLER_HPP_
#define BLANK_AI_AICONTROLLER_HPP_

#include "../app/IntervalTimer.hpp"
#include "../geometry/primitive.hpp"
#include "../world/EntityController.hpp"

#include <glm/glm.hpp>


namespace blank {

class AIState;
class Entity;
class GaloisLFSR;
class Player;
class World;

class AIController
: public EntityController {

public:
	AIController(World &, GaloisLFSR &);
	~AIController();

	void SetState(const AIState &);

	void Update(Entity &, float dt) override;

	glm::vec3 ControlForce(const Entity &, const EntityState &) const override;

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

	void EnterHalt() noexcept;
	void ExitHalt() noexcept;
	bool IsHalted() const noexcept;
	void SetHaltSpeed(float) noexcept;
	glm::vec3 GetHaltForce(const Entity &, const EntityState &) const noexcept;

	void StartAvoidingObstacles() noexcept;
	void StopAvoidingObstacles() noexcept;
	bool IsAvoidingObstacles() const noexcept;
	glm::vec3 GetObstacleAvoidanceForce(const Entity &, const EntityState &) const noexcept;

	void StartFleeing() noexcept;
	void StopFleeing() noexcept;
	bool IsFleeing() const noexcept;
	void SetFleeTarget(Entity &) noexcept;
	void SetFleeSpeed(float) noexcept;
	Entity &GetFleeTarget() noexcept;
	const Entity &GetFleeTarget() const noexcept;
	glm::vec3 GetFleeForce(const Entity &, const EntityState &) const noexcept;

	void StartSeeking() noexcept;
	void StopSeeking() noexcept;
	bool IsSeeking() const noexcept;
	void SetSeekTarget(Entity &) noexcept;
	void SetSeekSpeed(float) noexcept;
	Entity &GetSeekTarget() noexcept;
	const Entity &GetSeekTarget() const noexcept;
	glm::vec3 GetSeekForce(const Entity &, const EntityState &) const noexcept;

	void StartEvading() noexcept;
	void StopEvading() noexcept;
	bool IsEvading() const noexcept;
	void SetEvadeTarget(Entity &) noexcept;
	void SetEvadeSpeed(float) noexcept;
	Entity &GetEvadeTarget() noexcept;
	const Entity &GetEvadeTarget() const noexcept;
	glm::vec3 GetEvadeForce(const Entity &, const EntityState &) const noexcept;

	void StartPursuing() noexcept;
	void StopPursuing() noexcept;
	bool IsPursuing() const noexcept;
	void SetPursuitTarget(Entity &) noexcept;
	void SetPursuitSpeed(float) noexcept;
	Entity &GetPursuitTarget() noexcept;
	const Entity &GetPursuitTarget() const noexcept;
	glm::vec3 GetPursuitForce(const Entity &, const EntityState &) const noexcept;

	/// start wandering randomly
	void StartWandering() noexcept;
	void StopWandering() noexcept;
	bool IsWandering() const noexcept;
	/// change how wandering is performed
	/// the trajectory is modified by targetting a blip on a sphere
	/// in front of the entity which moves randomly
	/// the displacement is given (roughly) in units per second
	void SetWanderParams(
		float speed,
		float distance = 2.0f,
		float radius = 1.0f,
		float displacement = 1.0f
	) noexcept;
	glm::vec3 GetWanderForce(const Entity &, const EntityState &) const noexcept;

private:
	World &world;
	GaloisLFSR &random;
	const AIState *state;

	/// how far controlled entities can see
	float sight_dist;
	/// cosine of the half angle of FOV of controlled entities
	float sight_angle;

	FineTimer think_timer;
	FineTimer decision_timer;

	bool halted;
	float halt_speed;

	bool avoid_obstacles;
	AABB obstacle_box;
	glm::mat4 obstacle_transform;

	bool fleeing;
	Entity *flee_target;
	float flee_speed;

	bool seeking;
	Entity *seek_target;
	float seek_speed;

	bool evading;
	Entity *evade_target;
	float evade_speed;

	bool pursuing;
	Entity *pursuit_target;
	float pursuit_speed;

	bool wandering;
	glm::vec3 wander_pos;
	float wander_speed;
	float wander_dist;
	float wander_radius;
	float wander_disp;

};

}

#endif
