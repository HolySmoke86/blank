#include "RandomWalk.hpp"

#include "../world/Entity.hpp"


namespace blank {

RandomWalk::RandomWalk(Entity &e) noexcept
: entity(e)
, time_left(0) {

}


void RandomWalk::Update(int dt) noexcept {
	time_left -= dt;
	if (time_left > 0) return;
	time_left += 2500 + (rand() % 5000);

	constexpr float move_vel = 0.0005f;

	glm::vec3 new_vel = entity.Velocity();

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

	entity.Velocity(new_vel);
}

}
