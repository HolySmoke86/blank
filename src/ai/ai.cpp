#include "AIController.hpp"
#include "ChaseState.hpp"
#include "FleeState.hpp"
#include "IdleState.hpp"
#include "RoamState.hpp"

#include "../geometry/distance.hpp"
#include "../geometry/rotation.hpp"
#include "../rand/GaloisLFSR.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"
#include "../world/WorldCollision.hpp"

#include <cmath>
#include <limits>
#include <glm/glm.hpp>


namespace blank {

namespace {

ChaseState chase;
FleeState flee;
IdleState idle;
RoamState roam;

}

AIController::AIController(World &world, GaloisLFSR &rand)
: world(world)
, random(rand)
, state(&idle)
, sight_dist(64.0f)
, sight_angle(0.707f)
, think_timer(0.5f)
, decision_timer(1.0f)
, halted(false)
, halt_speed(1.0f)
, avoid_obstacles(true)
, obstacle_box{ glm::vec3(0.0f), glm::vec3(0.0f) }
, obstacle_transform(1.0f)
, fleeing(false)
, flee_target(nullptr)
, flee_speed(5.0f)
, seeking(false)
, seek_target(nullptr)
, seek_speed(5.0f)
, evading(false)
, evade_target(nullptr)
, evade_speed(5.0f)
, pursuing(false)
, pursuit_target(nullptr)
, pursuit_speed(5.0f)
, wandering(false)
, wander_pos(1.0f, 0.0f, 0.0f)
, wander_speed(1.0f)
, wander_dist(2.0f)
, wander_radius(1.5f)
, wander_disp(1.0f) {
	think_timer.Start();
	state->Enter(*this);
}

AIController::~AIController() {
	state->Exit(*this);
}

void AIController::SetState(const AIState &s) {
	state->Exit(*this);
	state = &s;
	state->Enter(*this);
}

void AIController::Update(Entity &e, float dt) {
	think_timer.Update(dt);
	decision_timer.Update(dt);
	state->Update(*this, e, dt);

	if (avoid_obstacles && e.Moving()) {
		obstacle_box = e.Bounds();
		obstacle_box.min.z = -e.Speed();
		obstacle_box.max.x = 0.0f;
		// our box is oriented for -Z velocity
		obstacle_transform = glm::mat4(find_rotation(glm::vec3(0.0f, 0.0f, -1.0f), e.Heading()));
		// and positioned relative to the entity's chunk
		obstacle_transform[3] = glm::vec4(e.GetState().pos.block, 1.0f);
	}

	if (wandering) {
		glm::vec3 displacement(
			random.SNorm() * wander_disp,
			random.SNorm() * wander_disp,
			random.SNorm() * wander_disp
		);
		if (!iszero(displacement)) {
			wander_pos = normalize(wander_pos + displacement * dt) * wander_radius;
		}
	}

	if (e.Moving()) {
		// orient head towards heading
		glm::vec3 heading(e.Heading());
		// only half pitch, so we don't crane our neck
		float tgt_pitch = std::atan(heading.y / length(glm::vec2(heading.x, heading.z))) * 0.5f;
		// always look straight ahead
		// maybe look at the pursuit target if there is one
		float tgt_yaw = 0.0f;
		e.SetHead(tgt_pitch, tgt_yaw);
		e.OrientBody(dt);
	}
}

glm::vec3 AIController::ControlForce(const Entity &entity, const EntityState &state) const {
	if (IsHalted()) {
		return GetHaltForce(entity, state);
	}
	glm::vec3 force(0.0f);
	if (IsAvoidingObstacles() && entity.Moving()) {
		if (MaxOutForce(force, GetObstacleAvoidanceForce(entity, state), entity.MaxControlForce())) {
			return force;
		}
	}
	if (IsFleeing()) {
		if (MaxOutForce(force, GetFleeForce(entity, state), entity.MaxControlForce())) {
			return force;
		}
	}
	if (IsSeeking()) {
		if (MaxOutForce(force, GetSeekForce(entity, state), entity.MaxControlForce())) {
			return force;
		}
	}
	if (IsEvading()) {
		if (MaxOutForce(force, GetEvadeForce(entity, state), entity.MaxControlForce())) {
			return force;
		}
	}
	if (IsPursuing()) {
		if (MaxOutForce(force, GetPursuitForce(entity, state), entity.MaxControlForce())) {
			return force;
		}
	}
	if (IsWandering()) {
		if (MaxOutForce(force, GetWanderForce(entity, state), entity.MaxControlForce())) {
			return force;
		}
	}
	return force;
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
		float dist = length(diff);
		if (dist > distance) continue;

		// FOV test, 45° in each direction
		if (dot(diff / dist, aim.dir) < sight_angle) {
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
	float dist = length(diff);
	if (dist > sight_dist || dot(diff / dist, aim.dir) < sight_angle) {
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
	decision_timer = FineTimer(minimum + variance * random.SNorm());
	decision_timer.Start();
}

bool AIController::DecisionDue() const noexcept {
	return decision_timer.HitOnce();
}

unsigned int AIController::Decide(unsigned int num_choices) noexcept {
	return random.Next<unsigned int>() % num_choices;
}

// halt

void AIController::EnterHalt() noexcept {
	halted = true;
}

void AIController::ExitHalt() noexcept {
	halted = false;
}

bool AIController::IsHalted() const noexcept {
	return halted;
}

void AIController::SetHaltSpeed(float speed) noexcept {
	halt_speed = speed;
}

glm::vec3 AIController::GetHaltForce(const Entity &, const EntityState &state) const noexcept {
	return Halt(state, halt_speed);
}

// obstacle avoidance

void AIController::StartAvoidingObstacles() noexcept {
	avoid_obstacles = true;
}

void AIController::StopAvoidingObstacles() noexcept {
	avoid_obstacles = false;
}

bool AIController::IsAvoidingObstacles() const noexcept {
	return avoid_obstacles;
}

namespace {

std::vector<WorldCollision> col;

}

glm::vec3 AIController::GetObstacleAvoidanceForce(const Entity &e, const EntityState &state) const noexcept {
	if (!e.Moving()) {
		return glm::vec3(0.0f);
	}
	col.clear();
	if (!world.Intersection(obstacle_box, obstacle_transform, e.ChunkCoords(), col)) {
		return glm::vec3(0.0f);
	}
	// find the nearest block
	WorldCollision *nearest = nullptr;
	glm::vec3 difference(0.0f);
	float distance = std::numeric_limits<float>::infinity();
	for (WorldCollision &c : col) {
		// diff points from block to state
		glm::vec3 diff = state.RelativePosition(c.ChunkPos()) - c.BlockCoords();
		float dist = length_squared(diff);
		if (dist < distance) {
			nearest = &c;
			difference = diff;
			distance = dist;
		}
	}
	if (!nearest) {
		// intersection test lied to us
		return glm::vec3(0.0f);
	}
	// and steer away from it
	// to_go is the distance between our position and the
	// point on the "velocity ray" closest to obstacle
	float to_go = dot(difference, e.Heading());
	// point is our future position if we keep going our way
	glm::vec3 point(e.GetState().pos.block + e.Heading() * to_go);
	// now steer away in the direction of (point - block)
	// with a magniture proportional to speed/distance
	return normalize(point - nearest->BlockCoords()) * (e.Speed() / std::sqrt(distance));
}

// flee

void AIController::StartFleeing() noexcept {
	fleeing = true;
}

void AIController::StopFleeing() noexcept {
	fleeing = false;
	if (flee_target) {
		flee_target->UnRef();
		flee_target = nullptr;
	}
}

bool AIController::IsFleeing() const noexcept {
	return fleeing && flee_target;
}

void AIController::SetFleeTarget(Entity &e) noexcept {
	if (flee_target) {
		flee_target->UnRef();
	}
	flee_target = &e;
	flee_target->Ref();
}

void AIController::SetFleeSpeed(float speed) noexcept {
	flee_speed = speed;
}

Entity &AIController::GetFleeTarget() noexcept {
	return *flee_target;
}

const Entity &AIController::GetFleeTarget() const noexcept {
	return *flee_target;
}

glm::vec3 AIController::GetFleeForce(const Entity &, const EntityState &state) const noexcept {
	return Flee(state, GetFleeTarget().GetState(), flee_speed, 2.0f);
}

// seek

void AIController::StartSeeking() noexcept {
	seeking = true;
}

void AIController::StopSeeking() noexcept {
	seeking = false;
	if (seek_target) {
		seek_target->UnRef();
		seek_target = nullptr;
	}
}

bool AIController::IsSeeking() const noexcept {
	return seeking && seek_target;
}

void AIController::SetSeekTarget(Entity &e) noexcept {
	if (seek_target) {
		seek_target->UnRef();
	}
	seek_target = &e;
	seek_target->Ref();
}

void AIController::SetSeekSpeed(float speed) noexcept {
	seek_speed = speed;
}

Entity &AIController::GetSeekTarget() noexcept {
	return *seek_target;
}

const Entity &AIController::GetSeekTarget() const noexcept {
	return *seek_target;
}

glm::vec3 AIController::GetSeekForce(const Entity &, const EntityState &state) const noexcept {
	return Seek(state, GetSeekTarget().GetState(), seek_speed, 2.0f);
}

// evade

void AIController::StartEvading() noexcept {
	evading = true;
}

void AIController::StopEvading() noexcept {
	evading = false;
	if (evade_target) {
		evade_target->UnRef();
		evade_target = nullptr;
	}
}

bool AIController::IsEvading() const noexcept {
	return evading && evade_target;
}

void AIController::SetEvadeTarget(Entity &e) noexcept {
	if (evade_target) {
		evade_target->UnRef();
	}
	evade_target = &e;
	evade_target->Ref();
}

void AIController::SetEvadeSpeed(float speed) noexcept {
	evade_speed = speed;
}

Entity &AIController::GetEvadeTarget() noexcept {
	return *evade_target;
}

const Entity &AIController::GetEvadeTarget() const noexcept {
	return *evade_target;
}

glm::vec3 AIController::GetEvadeForce(const Entity &, const EntityState &state) const noexcept{
	glm::vec3 cur_diff(state.Diff(GetEvadeTarget().GetState()));
	float time_estimate = length(cur_diff) / evade_speed;
	EntityState pred_state(GetEvadeTarget().GetState());
	pred_state.pos.block += pred_state.velocity * time_estimate;
	return Flee(state, pred_state, evade_speed, 2.0f);
}

// pursuit

void AIController::StartPursuing() noexcept {
	pursuing = true;
}

void AIController::StopPursuing() noexcept {
	pursuing = false;
	if (pursuit_target) {
		pursuit_target->UnRef();
		pursuit_target = nullptr;
	}
}

bool AIController::IsPursuing() const noexcept {
	return pursuing && pursuit_target;
}

void AIController::SetPursuitTarget(Entity &e) noexcept {
	if (pursuit_target) {
		pursuit_target->UnRef();
	}
	pursuit_target = &e;
	pursuit_target->Ref();
}

void AIController::SetPursuitSpeed(float speed) noexcept {
	pursuit_speed = speed;
}

Entity &AIController::GetPursuitTarget() noexcept {
	return *pursuit_target;
}

const Entity &AIController::GetPursuitTarget() const noexcept {
	return *pursuit_target;
}

glm::vec3 AIController::GetPursuitForce(const Entity &, const EntityState &state) const noexcept {
	glm::vec3 cur_diff(state.Diff(GetPursuitTarget().GetState()));
	float time_estimate = length(cur_diff) / pursuit_speed;
	EntityState pred_state(GetPursuitTarget().GetState());
	pred_state.pos.block += pred_state.velocity * time_estimate;
	return Seek(state, pred_state, pursuit_speed, 2.0f);
}

// wander

void AIController::StartWandering() noexcept {
	wandering = true;
}

void AIController::StopWandering() noexcept {
	wandering = false;
}

bool AIController::IsWandering() const noexcept {
	return wandering;
}

void AIController::SetWanderParams(
	float speed,
	float distance,
	float radius,
	float displacement
) noexcept {
	wander_speed = speed;
	wander_dist = distance;
	wander_radius = radius;
	wander_disp = displacement;
}

glm::vec3 AIController::GetWanderForce(const Entity &e, const EntityState &state) const noexcept {
	glm::vec3 wander_target(normalize(e.Heading() * wander_dist + wander_pos) * wander_speed);
	return TargetVelocity(wander_target, state, 0.5f);
}


// chase

void ChaseState::Enter(AIController &ctrl) const {
	ctrl.SetHaltSpeed(2.0f);
	ctrl.SetPursuitSpeed(4.0f);
	ctrl.StartPursuing();
}

void ChaseState::Update(AIController &ctrl, Entity &e, float dt) const {
	// check if target still alive and in sight
	if (ctrl.GetPursuitTarget().Dead()) {
		ctrl.SetState(idle);
		return;
	}
	if (!ctrl.LineOfSight(e, ctrl.GetPursuitTarget())) {
		ctrl.SetState(idle);
		return;
	}
	// halt if we're close enough, flee if we're too close
	float dist_sq = length_squared(e.AbsoluteDifference(ctrl.GetPursuitTarget()));
	if (dist_sq < 8.0f) {
		ctrl.SetFleeTarget(ctrl.GetPursuitTarget());
		ctrl.SetState(flee);
	} else if (dist_sq < 25.0f) {
		ctrl.EnterHalt();
	} else {
		ctrl.ExitHalt();
	}
}

void ChaseState::Exit(AIController &ctrl) const {
	ctrl.StopPursuing();
	ctrl.ExitHalt();
}

// flee

void FleeState::Enter(AIController &ctrl) const {
	ctrl.CueDecision(6.0f, 3.0f);
	ctrl.SetFleeSpeed(4.0f);
	ctrl.StartFleeing();
}

void FleeState::Update(AIController &ctrl, Entity &e, float dt) const {
	if (!ctrl.DecisionDue()) return;
	ctrl.SetState(idle);
}

void FleeState::Exit(AIController &ctrl) const {
	ctrl.StopFleeing();
}

// idle

void IdleState::Enter(AIController &ctrl) const {
	ctrl.SetHaltSpeed(0.5f);
	ctrl.EnterHalt();
	ctrl.SetWanderParams(0.001f, 1.1f);
	ctrl.CueDecision(10.0f, 5.0f);
}

void IdleState::Update(AIController &ctrl, Entity &e, float dt) const {
	if (ctrl.MayThink()) {
		const Player *player = ctrl.ClosestVisiblePlayer(e);
		if (player) {
			ctrl.SetPursuitTarget(player->GetEntity());
			ctrl.SetState(chase);
			return;
		}
	}

	if (!ctrl.DecisionDue()) return;

	unsigned int d = ctrl.Decide(10);
	if (d < 2) {
		// .2 chance to start going
		ctrl.SetState(roam);
	} else if (d < 5) {
		// .3 chance of looking around
		ctrl.ExitHalt();
		ctrl.StartWandering();
	} else {
		// .5 chance of doing nothing
		ctrl.StopWandering();
		ctrl.EnterHalt();
	}
	ctrl.CueDecision(10.0f, 5.0f);
}

void IdleState::Exit(AIController &ctrl) const {
	ctrl.ExitHalt();
	ctrl.StopWandering();
}

// roam

void RoamState::Enter(AIController &ctrl) const {
	ctrl.SetWanderParams(1.0f);
	ctrl.StartWandering();
	ctrl.CueDecision(10.0f, 5.0f);
}

void RoamState::Update(AIController &ctrl, Entity &e, float dt) const {
	if (ctrl.MayThink()) {
		const Player *player = ctrl.ClosestVisiblePlayer(e);
		if (player) {
			ctrl.SetPursuitTarget(player->GetEntity());
			ctrl.SetState(chase);
			return;
		}
	}

	if (!ctrl.DecisionDue()) return;

	unsigned int d = ctrl.Decide(10);
	if (d == 0) {
		// .1 chance of idling
		ctrl.SetState(idle);
	}
	ctrl.CueDecision(10.0f, 5.0f);
}

void RoamState::Exit(AIController &ctrl) const {
	ctrl.StopWandering();
}

}
