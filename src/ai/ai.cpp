#include "AIController.hpp"

#include "../model/geometry.hpp"
#include "../rand/GaloisLFSR.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"
#include "../world/WorldCollision.hpp"

#include <cmath>
#include <limits>
#include <glm/glm.hpp>


namespace blank {

AIController::AIController(GaloisLFSR &rand)
: random(rand)
, chase_speed(2.0f)
, flee_speed(-5.0f)
, stop_dist(10.0f)
, flee_dist(5.0f)
, wander_pos(1.0f, 0.0f, 0.0f)
, wander_dist(2.0f)
, wander_radius(1.0f)
, wander_disp(1.0f)
, wander_speed(1.0f) {

}

AIController::~AIController() {

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
	return (Heading(state) * wander_dist + wander_pos) * wander_speed;
}

glm::vec3 AIController::Heading(const EntityState &state) noexcept {
	if (dot(state.velocity, state.velocity) > std::numeric_limits<float>::epsilon()) {
		return normalize(state.velocity);
	} else {
		float cp = std::cos(state.pitch);
		return glm::vec3(std::cos(state.yaw) * cp, std::sin(state.yaw) * cp, std::sin(state.pitch));
	}
}

}
