#ifndef BLANK_AI_AICONTROLLER_HPP_
#define BLANK_AI_AICONTROLLER_HPP_

#include "../world/EntityController.hpp"

#include <glm/glm.hpp>


namespace blank {

class GaloisLFSR;

class AIController
: public EntityController {

public:
	explicit AIController(GaloisLFSR &);
	~AIController();

	void Update(Entity &, float dt) override;

	glm::vec3 ControlForce(const EntityState &) const override;

	static glm::vec3 Heading(const EntityState &) noexcept;

private:
	GaloisLFSR &random;

	float chase_speed;
	float flee_speed;
	float stop_dist;
	float flee_dist;

	glm::vec3 wander_pos;
	float wander_dist;
	float wander_radius;
	float wander_disp;
	float wander_speed;

};

}

#endif
