#include "AIController.hpp"
#include "IdleState.hpp"

#include "../model/geometry.hpp"
#include "../rand/GaloisLFSR.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"
#include "../world/WorldCollision.hpp"

#include <cmath>
#include <limits>
#include <glm/glm.hpp>


namespace blank {

namespace {

IdleState idle;

}

AIController::AIController(GaloisLFSR &rand)
: random(rand)
, state(&idle)
, flee_target(nullptr)
, flee_speed(-5.0f)
, seek_target(nullptr)
, seek_speed(-5.0f)
, wandering(false)
, wander_pos(1.0f, 0.0f, 0.0f)
, wander_dist(2.0f)
, wander_radius(1.0f)
, wander_disp(1.0f)
, wander_speed(1.0f) {
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
	// movement: for now, wander only
	glm::vec3 displacement(
		random.SNorm() * wander_disp,
		random.SNorm() * wander_disp,
		random.SNorm() * wander_disp
	);
	if (dot(displacement, displacement) > std::numeric_limits<float>::epsilon()) {
		wander_pos = normalize(wander_pos + displacement * dt) * wander_radius;
	}

	if (e.Moving()) {
		// orient head towards heading
		glm::vec3 heading(Heading(e.GetState()));
		float tgt_pitch = std::atan(heading.y / length(glm::vec2(heading.x, heading.z)));
		float tgt_yaw = std::atan2(-heading.x, -heading.z);
		e.SetHead(tgt_pitch, tgt_yaw);
	}
}

glm::vec3 AIController::ControlForce(const EntityState &state) const {
	glm::vec3 force(0.0f);
	if (IsFleeing()) {
		glm::vec3 diff(GetFleeTarget().GetState().Diff(state));
		if (dot(diff, diff) > std::numeric_limits<float>::epsilon()) {
			force += normalize(diff) * flee_speed;
		}
	}
	if (IsSeeking()) {
		glm::vec3 diff(state.Diff(GetSeekTarget().GetState()));
		if (dot(diff, diff) > std::numeric_limits<float>::epsilon()) {
			force += normalize(diff) * seek_speed;
		}
	}
	if (wandering) {
		force += (Heading(state) * wander_dist + wander_pos) * wander_speed;
	}
	return force;
}

glm::vec3 AIController::Heading(const EntityState &state) noexcept {
	if (dot(state.velocity, state.velocity) > std::numeric_limits<float>::epsilon()) {
		return normalize(state.velocity);
	} else {
		float cp = std::cos(state.pitch);
		return glm::vec3(std::cos(state.yaw) * cp, std::sin(state.yaw) * cp, std::sin(state.pitch));
	}
}

void AIController::StartFleeing(const Entity &e, float speed) noexcept {
	flee_target = &e;
	flee_speed = speed;
}

void AIController::StopFleeing() noexcept {
	flee_target = nullptr;
}

bool AIController::IsFleeing() const noexcept {
	return flee_target;
}

const Entity &AIController::GetFleeTarget() const noexcept {
	return *flee_target;
}

void AIController::StartSeeking(const Entity &e, float speed) noexcept {
	seek_target = &e;
	seek_speed = speed;
}

void AIController::StopSeeking() noexcept {
	seek_target = nullptr;
}

bool AIController::IsSeeking() const noexcept {
	return seek_target;
}

const Entity &AIController::GetSeekTarget() const noexcept {
	return *seek_target;
}

void AIController::StartWandering() noexcept {
	wandering = true;
}
void AIController::StopWandering() noexcept {
	wandering = false;
}


void IdleState::Enter(AIController &ctrl) const {
	ctrl.StartWandering();
}

void IdleState::Update(AIController &ctrl, Entity &e, float dt) const {
}

void IdleState::Exit(AIController &ctrl) const {
	ctrl.StopWandering();
}

}
