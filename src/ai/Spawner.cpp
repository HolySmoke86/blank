#include "Spawner.hpp"

#include "Chaser.hpp"
#include "RandomWalk.hpp"
#include "../world/BlockLookup.hpp"
#include "../world/BlockType.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"


namespace blank {

Spawner::Spawner(World &world)
: world(world)
, controllers()
, timer(64)
, despawn_range(128 * 128)
, spawn_distance(16 * 16)
, max_entities(16)
, chunk_range(4) {
	EntityModel::Buffer buf;
	for (size_t i = 0; i < 14; ++i) {
		world.BlockTypes()[i + 1].FillEntityModel(buf);
		models[i].Update(buf);
		buf.Clear();
	}

	timer.Start();
	Spawn(world.Player().ChunkCoords(), { 0.5f, 0.5f, 0.5f });
}

Spawner::~Spawner() {
	for (auto &ctrl : controllers) {
		delete ctrl;
	}
}


void Spawner::Update(int dt) {
	CheckDespawn();
	timer.Update(dt);
	if (timer.Hit()) {
		TrySpawn();
	}
	for (auto &ctrl : controllers) {
		ctrl->Update(dt);
	}
}


void Spawner::CheckDespawn() noexcept {
	const Entity &reference = world.Player();
	for (auto iter = controllers.begin(), end = controllers.end(); iter != end;) {
		Entity &e = (*iter)->Controlled();
		glm::vec3 diff(reference.AbsoluteDifference(e));
		if (dot(diff, diff) > despawn_range) {
			e.Remove();
			delete *iter;
			iter = controllers.erase(iter);
		} else {
			++iter;
		}
	}
}

void Spawner::TrySpawn() {
	if (controllers.size() >= max_entities) return;

	glm::ivec3 chunk(
		(rand() % (chunk_range * 2 + 1)) - chunk_range,
		(rand() % (chunk_range * 2 + 1)) - chunk_range,
		(rand() % (chunk_range * 2 + 1)) - chunk_range
	);

	glm::ivec3 pos(
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

	// check if the spawn block and the one above it are loaded and inhabitable
	BlockLookup spawn_block(&world.PlayerChunk(), chunk * Chunk::Extent() + pos);
	if (!spawn_block || spawn_block.GetType().collide_block) {
		return;
	}

	BlockLookup head_block(spawn_block.Next(Block::FACE_UP));
	if (!head_block || head_block.GetType().collide_block) {
		return;
	}

	Spawn(world.Player().ChunkCoords() + chunk, glm::vec3(pos) + glm::vec3(0.5f));
}

void Spawner::Spawn(const glm::ivec3 &chunk, const glm::vec3 &pos) {
	glm::vec3 rot(0.000001f);
	rot.x *= (rand() % 1024);
	rot.y *= (rand() % 1024);
	rot.z *= (rand() % 1024);

	Entity &e = world.AddEntity();
	e.Name("spawned");
	e.Position(chunk, pos);
	e.Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	e.WorldCollidable(true);
	e.GetModel().SetNodeModel(&models[rand() % 14]);
	e.AngularVelocity(rot);
	Controller *ctrl;
	if (rand() % 2) {
		ctrl = new RandomWalk(e);
	} else {
		ctrl = new Chaser(world, e, world.Player());
	}
	controllers.emplace_back(ctrl);
}

}
