#include "Chaser.hpp"
#include "Controller.hpp"
#include "RandomWalk.hpp"

#include "../world/Entity.hpp"

#include <glm/glm.hpp>


namespace blank {

Chaser::Chaser(Entity &ctrl, Entity &tgt) noexcept
: Controller(ctrl)
, tgt(tgt)
, speed(0.002f)
, stop_dist(5 * 5)
, flee_dist(3 * 3) {

}

Chaser::~Chaser() {

}

void Chaser::Update(int dt) {
	glm::vec3 diff(Target().AbsoluteDifference(Controlled()));
	float dist = dot (diff, diff);
	// TODO: line of sight test
	if (dist > stop_dist) {
		Controlled().Velocity(normalize(diff) * speed);
	} else if (dist < flee_dist) {
		Controlled().Velocity(normalize(diff) * -speed);
	} else {
		Controlled().Velocity(glm::vec3(0.0f));
	}
}


Controller::Controller(Entity &e) noexcept
: entity(e) {

}

Controller::~Controller() {

}


RandomWalk::RandomWalk(Entity &e) noexcept
: Controller(e)
, time_left(0) {

}

RandomWalk::~RandomWalk() {

}

void RandomWalk::Update(int dt) {
	time_left -= dt;
	if (time_left > 0) return;
	time_left += 2500 + (rand() % 5000);

	constexpr float move_vel = 0.0005f;

	glm::vec3 new_vel = Controlled().Velocity();

	switch (rand() % 9) {
		case 0:
			new_vel.x = -move_vel;
			break;
		case 1:
			new_vel.x = 0.0f;
			break;
		case 2:
			new_vel.x = move_vel;
			break;
		case 3:
			new_vel.y = -move_vel;
			break;
		case 4:
			new_vel.y = 0.0f;
			break;
		case 5:
			new_vel.y = move_vel;
			break;
		case 6:
			new_vel.z = -move_vel;
			break;
		case 7:
			new_vel.z = 0.0f;
			break;
		case 8:
			new_vel.z = move_vel;
			break;
	}

	Controlled().Velocity(new_vel);
}

}
