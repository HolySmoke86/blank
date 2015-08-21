#include "Chaser.hpp"
#include "Controller.hpp"
#include "RandomWalk.hpp"

#include "../model/geometry.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"
#include "../world/WorldCollision.hpp"

#include <glm/glm.hpp>


namespace blank {

Chaser::Chaser(World &world, Entity &ctrl, Entity &tgt) noexcept
: Controller(ctrl)
, world(world)
, tgt(tgt)
, chase_speed(0.002f)
, flee_speed(-0.005f)
, stop_dist(10)
, flee_dist(5) {

}

Chaser::~Chaser() {

}

void Chaser::Update(int dt) {
	glm::vec3 diff(Target().AbsoluteDifference(Controlled()));
	float dist = length(diff);
	glm::vec3 norm_diff(diff / dist);

	bool line_of_sight = true;
	Ray aim{Target().Position() - diff, norm_diff};
	WorldCollision coll;
	if (world.Intersection(aim, glm::mat4(1.0f), Target().ChunkCoords(), coll)) {
		line_of_sight = coll.depth > dist;
	}

	if (!line_of_sight) {
		Controlled().Velocity(glm::vec3(0.0f));
	} else if (dist > stop_dist) {
		Controlled().Velocity(norm_diff * chase_speed);
	} else if (dist < flee_dist) {
		Controlled().Velocity(norm_diff * flee_speed);
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
