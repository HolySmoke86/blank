#include "Spawner.hpp"

#include "Chaser.hpp"
#include "RandomWalk.hpp"
#include "../model/shapes.hpp"
#include "../world/BlockLookup.hpp"
#include "../world/BlockType.hpp"
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
	{
		CuboidShape shape({{ -0.25f, -0.5f, -0.25f }, { 0.25f, 0.5f, 0.25f }});
		shape.Vertices(buf, 1.0f);
		buf.colors.resize(shape.VertexCount(), { 1.0f, 1.0f, 0.0f });
		models[0].Update(buf);
	}
	{
		CuboidShape shape({{ -0.5f, -0.25f, -0.5f }, { 0.5f, 0.25f, 0.5f }});
		buf.Clear();
		shape.Vertices(buf, 2.0f);
		buf.colors.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		models[1].Update(buf);
	}
	{
		StairShape shape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }}, { 0.4f, 0.4f });
		buf.Clear();
		shape.Vertices(buf, 3.0f);
		buf.colors.resize(shape.VertexCount(), { 1.0f, 0.0f, 1.0f });
		models[2].Update(buf);
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
	e.GetModel().SetNodeModel(&models[rand() % 3]);
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
