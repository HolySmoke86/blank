#include "Spawner.hpp"

#include "RandomWalk.hpp"
#include "../world/BlockType.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"


namespace blank {

Spawner::Spawner(World &world)
: world(world)
, controllers()
, timer(8096)
, despawn_range(128 * 128)
, spawn_distance(16 * 16)
, max_entities(16)
, chunk_range(4) {
	timer.Start();
	Spawn(world.Player().ChunkCoords(), { 0.5f, 0.5f, 0.5f });
}

Spawner::~Spawner() {

}


void Spawner::Update(int dt) {
	CheckDespawn();
	timer.Update(dt);
	if (timer.Hit()) {
		TrySpawn();
	}
	for (auto &ctrl : controllers) {
		ctrl.Update(dt);
	}
}


void Spawner::CheckDespawn() noexcept {
	const Entity &reference = world.Player();
	for (auto iter = controllers.begin(), end = controllers.end(); iter != end;) {
		Entity &e = iter->Controlled();
		glm::vec3 diff(reference.AbsoluteDifference(e));
		if (dot(diff, diff) > despawn_range) {
			e.Remove();
			iter = controllers.erase(iter);
		} else {
			++iter;
		}
	}
}

void Spawner::TrySpawn() {
	if (controllers.size() >= max_entities) return;

	glm::tvec3<int> chunk(
		(rand() % (chunk_range * 2 + 1)) - chunk_range,
		(rand() % (chunk_range * 2 + 1)) - chunk_range,
		(rand() % (chunk_range * 2 + 1)) - chunk_range
	);

	glm::tvec3<int> pos(
		rand() % Chunk::width,
		rand() % Chunk::height,
		rand() % Chunk::depth
	);


	// distance check
	glm::vec3 diff(glm::vec3(chunk * Chunk::Extent() - pos) + world.Player().Position());
	float dist = dot(diff, diff);
	if (dist > despawn_range || dist < spawn_distance) {
		return;
	}

	// block check
	// TODO: avoid force load, abort spawn if chunk unavailble
	Chunk &tgt_chunk = world.Next(world.PlayerChunk(), chunk);
	// TODO: don't use visibility for spawn check
	//       also, check for more than one block space
	if (tgt_chunk.Type(tgt_chunk.BlockAt(pos)).visible) {
		return;
	}

	Spawn(world.Player().ChunkCoords() + chunk, glm::vec3(pos) + glm::vec3(0.5f));
}

void Spawner::Spawn(const glm::tvec3<int> &chunk, const glm::vec3 &pos) {
	glm::vec3 color(rand() % 6, rand() % 6, rand() % 6);
	color = color * 0.15f + 0.25f;

	glm::vec3 rot(0.000001f);
	rot.x *= (rand() % 32);
	rot.y *= (rand() % 32);
	rot.z *= (rand() % 32);

	Entity &e = world.AddEntity();
	e.Name("test");
	e.Position(chunk, pos);
	e.Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	e.WorldCollidable(true);
	e.SetShape(world.BlockTypes()[1].shape, color);
	e.AngularVelocity(glm::quat(glm::vec3{ 0.00001f, 0.000006f, 0.000013f }));
	controllers.emplace_back(e);
}

}