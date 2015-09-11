#include "Spawner.hpp"

#include "Chaser.hpp"
#include "RandomWalk.hpp"
#include "../model/CompositeModel.hpp"
#include "../model/Skeletons.hpp"
#include "../world/BlockLookup.hpp"
#include "../world/BlockType.hpp"
#include "../world/ChunkIndex.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"


namespace blank {

Spawner::Spawner(World &world, Skeletons &skeletons, std::uint64_t seed)
: world(world)
, skeletons(skeletons)
, controllers()
, random(seed)
, timer(64)
, despawn_range(128 * 128)
, spawn_distance(16 * 16)
, max_entities(16)
, chunk_range(4) {
	timer.Start();
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
	const auto &refs = world.Players();
	for (auto iter = controllers.begin(), end = controllers.end(); iter != end;) {
		Entity &e = (*iter)->Controlled();
		if (e.Dead()) {
			delete *iter;
			iter = controllers.erase(iter);
			end = controllers.end();
			continue;
		}
		bool safe = false;
		for (const Player &ref : refs) {
			glm::vec3 diff(ref.entity->AbsoluteDifference(e));
			if (dot(diff, diff) < despawn_range) {
				safe = true;
				break;
			}
		}
		if (!safe) {
			e.Kill();
			delete *iter;
			iter = controllers.erase(iter);
			end = controllers.end();
		} else {
			++iter;
		}
	}
}

void Spawner::TrySpawn() {
	if (controllers.size() >= max_entities) return;

	// select random player to punish
	auto &players = world.Players();
	if (players.size() == 0) return;
	const Player &player = players[random.Next<unsigned short>() % players.size()];

	int index = random.Next<unsigned int>() % player.chunks->TotalChunks();

	glm::ivec3 chunk(player.chunks->PositionOf(index));

	glm::ivec3 pos(
		random.Next<unsigned char>() % Chunk::width,
		random.Next<unsigned char>() % Chunk::height,
		random.Next<unsigned char>() % Chunk::depth
	);

	// distance check
	//glm::vec3 diff(glm::vec3(chunk * Chunk::Extent() - pos) + player.entity->Position());
	//float dist = dot(diff, diff);
	//if (dist > despawn_range || dist < spawn_distance) {
	//	return;
	//}

	// check if the spawn block and the one above it are loaded and inhabitable
	BlockLookup spawn_block((*player.chunks)[index], pos);
	if (!spawn_block || spawn_block.GetType().collide_block) {
		return;
	}

	BlockLookup head_block(spawn_block.Next(Block::FACE_UP));
	if (!head_block || head_block.GetType().collide_block) {
		return;
	}

	Spawn(*player.entity, chunk, glm::vec3(pos) + glm::vec3(0.5f));
}

void Spawner::Spawn(Entity &reference, const glm::ivec3 &chunk, const glm::vec3 &pos) {
	glm::vec3 rot(0.000001f);
	rot.x *= (random.Next<unsigned short>() % 1024);
	rot.y *= (random.Next<unsigned short>() % 1024);
	rot.z *= (random.Next<unsigned short>() % 1024);

	Entity &e = world.AddEntity();
	e.Position(chunk, pos);
	e.Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	e.WorldCollidable(true);
	skeletons[random.Next<unsigned char>() % skeletons.Size()].Instantiate(e.GetModel());
	e.AngularVelocity(rot);
	Controller *ctrl;
	if (random()) {
		ctrl = new RandomWalk(e, random.Next<std::uint64_t>());
		e.Name("spawned walker");
	} else {
		ctrl = new Chaser(world, e, reference);
		e.Name("spawned chaser");
	}
	controllers.emplace_back(ctrl);
}

}
