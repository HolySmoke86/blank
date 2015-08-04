#include "Spawner.hpp"

#include "RandomWalk.hpp"
#include "../world/BlockType.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"


namespace blank {

Spawner::Spawner(World &world)
: world(world)
, controllers() {
	Spawn({ 0.0f, 0.0f, 0.0f });
}

Spawner::~Spawner() {

}


void Spawner::Update(int dt) {
	for (auto &ctrl : controllers) {
		ctrl.Update(dt);
	}
}


void Spawner::Spawn(const glm::vec3 &pos) {
	Entity &e = world.AddEntity();
	e.Name("test");
	e.Position(pos);
	e.Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	e.WorldCollidable(true);
	e.SetShape(world.BlockTypes()[1].shape, { 1.0f, 1.0f, 0.0f });
	e.AngularVelocity(glm::quat(glm::vec3{ 0.00001f, 0.000006f, 0.000013f }));
	controllers.emplace_back(e);
}

}
