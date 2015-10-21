#include "Spawner.hpp"

#include "Chaser.hpp"
#include "RandomWalk.hpp"
#include "../model/Model.hpp"
#include "../model/ModelRegistry.hpp"
#include "../rand/GaloisLFSR.hpp"
#include "../world/BlockLookup.hpp"
#include "../world/BlockType.hpp"
#include "../world/ChunkIndex.hpp"
#include "../world/Entity.hpp"
#include "../world/World.hpp"

#include <iostream>

using namespace std;


namespace blank {

Spawner::Spawner(World &world, ModelRegistry &models, GaloisLFSR &rand)
: world(world)
, models(models)
, controllers()
, random(rand)
, timer(64)
, despawn_range(128 * 128)
, spawn_distance(16 * 16)
, max_entities(16)
, chunk_range(4)
, model_offset(0)
, model_length(models.size()) {
	timer.Start();
}

Spawner::~Spawner() {
	for (auto &ctrl : controllers) {
		delete ctrl;
	}
}


void Spawner::LimitModels(size_t begin, size_t end) {
	if (begin >= models.size() || end > models.size() || begin >= end) {
		cout << "warning, models limit out of bounds or invalid range given" << endl;
	} else {
		model_offset = begin;
		model_length = end - begin;
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
			glm::vec3 diff(ref.GetEntity().AbsoluteDifference(e));
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
	if (controllers.size() >= max_entities || model_length == 0) return;

	// select random player to punish
	auto &players = world.Players();
	if (players.size() == 0) return;
	size_t player_num = random.Next<unsigned short>() % players.size();
	auto i = players.begin(), end = players.end();
	for (; player_num > 0 && i != end; ++i, --player_num) {
	}
	const Player &player = *i;

	BlockLookup spawn_block(player.GetChunks().RandomBlock(random));

	// distance check
	//glm::vec3 diff(glm::vec3(chunk * Chunk::Extent() - pos) + player.entity->Position());
	//float dist = dot(diff, diff);
	//if (dist > despawn_range || dist < spawn_distance) {
	//	return;
	//}

	// check if the spawn block and the one above it are loaded and inhabitable
	if (!spawn_block || spawn_block.GetType().collide_block) {
		return;
	}

	BlockLookup head_block(spawn_block.Next(Block::FACE_UP));
	if (!head_block || head_block.GetType().collide_block) {
		return;
	}

	Spawn(player.GetEntity(), spawn_block.GetChunk().Position(), spawn_block.GetBlockCoords());
}

void Spawner::Spawn(Entity &reference, const glm::ivec3 &chunk, const glm::vec3 &pos) {
	Entity &e = world.AddEntity();
	e.Position(chunk, pos);
	e.Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	e.WorldCollidable(true);
	RandomModel().Instantiate(e.GetModel());
	Controller *ctrl;
	if (random()) {
		ctrl = new RandomWalk(e, random.Next<std::uint64_t>());
		e.Name("walker");
	} else {
		ctrl = new Chaser(world, e, reference);
		e.Name("chaser");
	}
	controllers.emplace_back(ctrl);
}

Model &Spawner::RandomModel() noexcept {
	std::size_t offset = (random.Next<std::size_t>() % model_length) + model_offset;
	return models[offset];
}

}
