#include "AIController.hpp"
#include "ChaseState.hpp"
#include "FleeState.hpp"
#include "IdleState.hpp"
#include "RoamState.hpp"

#include "../geometry/distance.hpp"
#include "../geometry/rotation.hpp"
#include "../graphics/glm.hpp"
#include "../rand/GaloisLFSR.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"
#include "../world/WorldCollision.hpp"

#include <cmath>
#include <limits>


namespace blank {

namespace {

ChaseState chase;
FleeState flee;
IdleState idle;
RoamState roam;

}

AIController::AIController(World &world, Entity &entity)
: world(world)
, state(&idle)
, sight_dist(64.0f)
, sight_angle(0.707f)
, think_timer(0.5f)
, decision_timer(1.0f) {
	think_timer.Start();
	state->Enter(*this, entity);
}

AIController::~AIController() {
	// ignore this for now
	// state->Exit(*this, entity);
}

void AIController::SetState(const AIState &s, Entity &entity) {
	state->Exit(*this, entity);
	state = &s;
	state->Enter(*this, entity);
}

void AIController::Update(Entity &e, float dt) {
	think_timer.Update(dt);
	decision_timer.Update(dt);
	state->Update(*this, e, dt);

	if (e.Moving()) {
		// orient head towards heading
		glm::vec3 heading(e.Heading());
		// only half pitch, so we don't crane our neck
		float tgt_pitch = std::atan(heading.y / glm::length(glm::vec2(heading.x, heading.z))) * 0.5f;
		// always look straight ahead
		// maybe look at the pursuit target if there is one
		float tgt_yaw = 0.0f;
		e.SetHead(tgt_pitch, tgt_yaw);
		e.OrientBody(dt);
	}
}

Player *AIController::ClosestVisiblePlayer(const Entity &e) noexcept {
	Player *target = nullptr;
	float distance = sight_dist;
	const glm::ivec3 &reference(e.ChunkCoords());
	Ray aim(e.Aim(reference));
	for (Player &p : world.Players()) {
		const Entity &pe = p.GetEntity();

		// distance test
		const glm::vec3 diff(pe.AbsoluteDifference(e));
		float dist = glm::length(diff);
		if (dist > distance) continue;

		// FOV test, 45° in each direction
		if (glm::dot(diff / dist, aim.dir) < sight_angle) {
			continue;
		}

		// LOS test, assumes all entities are see-through
		WorldCollision col;
		if (world.Intersection(aim, reference, col) && col.depth < dist) {
			continue;
		}

		// we got a match
		target = &p;
		distance = dist;
	}
	return target;
}

bool AIController::LineOfSight(const Entity &from, const Entity &to) const noexcept {
	const glm::ivec3 &reference(from.ChunkCoords());
	Ray aim(from.Aim(reference));
	const glm::vec3 diff(to.AbsoluteDifference(from));
	float dist = glm::length(diff);
	if (dist > sight_dist || glm::dot(diff / dist, aim.dir) < sight_angle) {
		return false;
	}
	WorldCollision col;
	if (world.Intersection(aim, reference, col) && col.depth < dist) {
		return false;
	}
	return true;
}

// think

bool AIController::MayThink() const noexcept {
	return think_timer.Hit();
}

void AIController::SetThinkInterval(float i) noexcept {
	think_timer = FineTimer(i);
	think_timer.Start();
}

// decide

void AIController::CueDecision(
	float minimum,
	float variance
) noexcept {
	decision_timer = FineTimer(minimum + variance * world.Random().SNorm());
	decision_timer.Start();
}

bool AIController::DecisionDue() const noexcept {
	return decision_timer.HitOnce();
}

unsigned int AIController::Decide(unsigned int num_choices) noexcept {
	return world.Random().Next<unsigned int>() % num_choices;
}


// chase

void ChaseState::Enter(AIController &, Entity &e) const {
	e.GetSteering()
		.SetAcceleration(5.0f)
		.SetSpeed(4.0f)
		.Enable(Steering::PURSUE_TARGET)
	;
}

void ChaseState::Update(AIController &ctrl, Entity &e, float) const {
	Steering &steering = e.GetSteering();
	// check if target still alive and in sight
	if (
		!steering.HasTargetEntity() || // lost
		steering.GetTargetEntity().Dead() || // dead
		!ctrl.LineOfSight(e, steering.GetTargetEntity()) // escaped
	) {
		steering.ClearTargetEntity();
		ctrl.SetState(idle, e);
		return;
	}
	// halt if we're close enough, flee if we're too close
	float dist_sq = glm::length2(e.AbsoluteDifference(steering.GetTargetEntity()));
	if (dist_sq < 8.0f) {
		ctrl.SetState(flee, e);
	} else if (dist_sq < 25.0f) {
		steering.Enable(Steering::HALT).Disable(Steering::PURSUE_TARGET);
	} else {
		steering.Enable(Steering::PURSUE_TARGET).Disable(Steering::HALT);
	}
}

void ChaseState::Exit(AIController &, Entity &e) const {
	e.GetSteering().Disable(Steering::HALT | Steering::PURSUE_TARGET);
}

// flee

void FleeState::Enter(AIController &ctrl, Entity &e) const {
	e.GetSteering()
		.SetAcceleration(5.0f)
		.SetSpeed(4.0f)
		.Enable(Steering::EVADE_TARGET)
	;
	ctrl.CueDecision(6.0f, 3.0f);
}

void FleeState::Update(AIController &ctrl, Entity &e, float) const {
	if (!ctrl.DecisionDue()) return;
	ctrl.SetState(idle, e);
}

void FleeState::Exit(AIController &, Entity &e) const {
	e.GetSteering().Disable(Steering::EVADE_TARGET);
}

// idle

void IdleState::Enter(AIController &ctrl, Entity &e) const {
	e.GetSteering()
		.SetAcceleration(0.5f)
		.SetSpeed(0.01f)
		.Enable(Steering::HALT)
		.SetWanderParams(1.0f, 1.1f, 1.0f)
	;
	ctrl.CueDecision(10.0f, 5.0f);
}

void IdleState::Update(AIController &ctrl, Entity &e, float) const {
	if (ctrl.MayThink()) {
		const Player *player = ctrl.ClosestVisiblePlayer(e);
		if (player) {
			e.GetSteering().SetTargetEntity(player->GetEntity());
			ctrl.SetState(chase, e);
			return;
		}
	}

	if (!ctrl.DecisionDue()) return;

	unsigned int d = ctrl.Decide(10);
	if (d < 2) {
		// .2 chance to start going
		ctrl.SetState(roam, e);
	} else if (d < 5) {
		// .3 chance of looking around
		e.GetSteering().Disable(Steering::HALT).Enable(Steering::WANDER);
	} else {
		// .5 chance of doing nothing
		e.GetSteering().Disable(Steering::WANDER).Enable(Steering::HALT);
	}
	ctrl.CueDecision(10.0f, 5.0f);
}

void IdleState::Exit(AIController &, Entity &e) const {
	e.GetSteering().Disable(Steering::HALT | Steering::WANDER);
}

// roam

void RoamState::Enter(AIController &ctrl, Entity &e) const {
	e.GetSteering()
		.SetAcceleration(0.5f)
		.SetSpeed(1.0f)
		.SetWanderParams(1.0f, 2.0f, 1.0f)
		.Enable(Steering::WANDER)
	;
	ctrl.CueDecision(10.0f, 5.0f);
}

void RoamState::Update(AIController &ctrl, Entity &e, float) const {
	if (ctrl.MayThink()) {
		const Player *player = ctrl.ClosestVisiblePlayer(e);
		if (player) {
			e.GetSteering().SetTargetEntity(player->GetEntity());
			ctrl.SetState(chase, e);
			return;
		}
	}

	if (!ctrl.DecisionDue()) return;

	unsigned int d = ctrl.Decide(10);
	if (d == 0) {
		// .1 chance of idling
		ctrl.SetState(idle, e);
	}
	ctrl.CueDecision(10.0f, 5.0f);
}

void RoamState::Exit(AIController &, Entity &e) const {
	e.GetSteering().Disable(Steering::WANDER);
}

}
