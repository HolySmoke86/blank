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
	tgt.Ref();
}

Chaser::~Chaser() {
	tgt.UnRef();
}

void Chaser::Update(int dt) {
	if (Target().Dead()) {
		Controlled().Kill();
		return;
	}

	glm::vec3 diff(Target().AbsoluteDifference(Controlled()));
	float dist = length(diff);
	if (dist < std::numeric_limits<float>::epsilon()) {
		Controlled().Velocity(glm::vec3(0.0f));
		return;
	}
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
	entity.Ref();
}

Controller::~Controller() {
	entity.UnRef();
}


RandomWalk::RandomWalk(Entity &e, std::uint64_t seed) noexcept
: Controller(e)
, random(seed)
, start_vel(e.Velocity())
, target_vel(start_vel)
, start_rot(e.AngularVelocity())
, target_rot(start_rot)
, switch_time(0)
, lerp_max(1.0f)
, lerp_time(0.0f) {

}

RandomWalk::~RandomWalk() {

}

void RandomWalk::Update(int dt) {
	switch_time -= dt;
	lerp_time -= dt;
	if (switch_time < 0) {
		switch_time += 2500 + (random.Next<unsigned short>() % 5000);
		lerp_max = 1500 + (random.Next<unsigned short>() % 1000);
		lerp_time = lerp_max;
		Change();
	} else if (lerp_time > 0) {
		float a = std::min(lerp_time / lerp_max, 1.0f);
		Controlled().Velocity(mix(target_vel, start_vel, a));
		Controlled().AngularVelocity(mix(target_rot, start_rot, a));
	} else {
		Controlled().Velocity(target_vel);
		Controlled().AngularVelocity(target_rot);
	}
}

void RandomWalk::Change() noexcept {
	start_vel = target_vel;
	start_rot = target_rot;

	constexpr float base = 0.000001f;

	target_vel.x = base * (random.Next<short>() % 1024);
	target_vel.y = base * (random.Next<short>() % 1024);
	target_vel.z = base * (random.Next<short>() % 1024);

	target_rot.x = base * (random.Next<short>() % 1024);
	target_rot.y = base * (random.Next<short>() % 1024);
	target_rot.z = base * (random.Next<short>() % 1024);
}

}
