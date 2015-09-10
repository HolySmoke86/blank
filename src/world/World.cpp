#include "World.hpp"

#include "ChunkIndex.hpp"
#include "EntityCollision.hpp"
#include "WorldCollision.hpp"
#include "../app/Assets.hpp"
#include "../graphics/Format.hpp"
#include "../graphics/Viewport.hpp"

#include <algorithm>
#include <limits>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

World::World(const BlockTypeRegistry &types, const Config &config)
: config(config)
, block_type(types)
, chunks(types)
// TODO: set spawn base and extent from config
, spawn_index(chunks.MakeIndex(Chunk::Pos(0, 0, 0), 3))
, players()
, entities()
, light_direction(config.light_direction)
, fog_density(config.fog_density) {

}

World::~World() {
	chunks.UnregisterIndex(spawn_index);
}


Player World::AddPlayer(const std::string &name) {
	for (Player &p : players) {
		if (p.entity->Name() == name) {
			return { nullptr, nullptr };
		}
	}
	Entity &entity = AddEntity();
	entity.Name(name);
	// TODO: load from save file here
	entity.Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	entity.WorldCollidable(true);
	entity.Position(config.spawn);
	ChunkIndex *index = &chunks.MakeIndex(entity.ChunkCoords(), 6);
	players.emplace_back(&entity, index);
	return players.back();
}

Player World::AddPlayer(const std::string &name, std::uint32_t id) {
	for (Player &p : players) {
		if (p.entity->Name() == name) {
			return { nullptr, nullptr };
		}
	}
	Entity *entity = AddEntity(id);
	if (!entity) {
		return { nullptr, nullptr };
	}
	entity->Name(name);
	// TODO: load from save file here
	entity->Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	entity->WorldCollidable(true);
	entity->Position(config.spawn);
	ChunkIndex *index = &chunks.MakeIndex(entity->ChunkCoords(), 6);
	players.emplace_back(entity, index);
	return players.back();
}

Entity &World::AddEntity() {
	if (entities.empty()) {
		entities.emplace_back();
		entities.back().ID(1);
		return entities.back();
	}
	if (entities.back().ID() < std::numeric_limits<std::uint32_t>::max()) {
		std::uint32_t id = entities.back().ID() + 1;
		entities.emplace_back();
		entities.back().ID(id);
		return entities.back();
	}
	std::uint32_t id = 1;
	auto position = entities.begin();
	auto end = entities.end();
	while (position != end && position->ID() == id) {
		++id;
		++position;
	}
	auto entity = entities.emplace(position);
	entity->ID(id);
	return *entity;
}

Entity *World::AddEntity(std::uint32_t id) {
	if (entities.empty() || entities.back().ID() < id) {
		entities.emplace_back();
		entities.back().ID(id);
		return &entities.back();
	}

	auto position = entities.begin();
	auto end = entities.end();
	while (position != end && position->ID() < id) {
		++position;
	}
	if (position != end && position->ID() == id) {
		return nullptr;
	}
	auto entity = entities.emplace(position);
	entity->ID(id);
	return &*entity;
}


namespace {

struct Candidate {
	Chunk *chunk;
	float dist;
};

bool CandidateLess(const Candidate &a, const Candidate &b) {
	return a.dist < b.dist;
}

std::vector<Candidate> candidates;

}

bool World::Intersection(
	const Ray &ray,
	const glm::mat4 &M,
	const Chunk::Pos &reference,
	WorldCollision &coll
) {
	candidates.clear();

	for (Chunk &cur_chunk : chunks) {
		float cur_dist;
		if (cur_chunk.Intersection(ray, M * cur_chunk.Transform(reference), cur_dist)) {
			candidates.push_back({ &cur_chunk, cur_dist });
		}
	}

	if (candidates.empty()) return false;

	std::sort(candidates.begin(), candidates.end(), CandidateLess);

	coll.chunk = nullptr;
	coll.block = -1;
	coll.depth = std::numeric_limits<float>::infinity();

	for (Candidate &cand : candidates) {
		if (cand.dist > coll.depth) continue;
		WorldCollision cur_coll;
		if (cand.chunk->Intersection(ray, M * cand.chunk->Transform(reference), cur_coll)) {
			if (cur_coll.depth < coll.depth) {
				coll = cur_coll;
			}
		}
	}

	return coll.chunk;
}

bool World::Intersection(
	const Ray &ray,
	const glm::mat4 &M,
	const Entity &reference,
	EntityCollision &coll
) {
	coll.entity = nullptr;
	coll.depth = std::numeric_limits<float>::infinity();
	for (Entity &cur_entity : entities) {
		if (&cur_entity == &reference) {
			continue;
		}
		float cur_dist;
		glm::vec3 cur_normal;
		if (blank::Intersection(ray, cur_entity.Bounds(), M * cur_entity.Transform(reference.ChunkCoords()), &cur_dist, &cur_normal)) {
			// TODO: fine grained check goes here? maybe?
			if (cur_dist < coll.depth) {
				coll.entity = &cur_entity;
				coll.depth = cur_dist;
				coll.normal = cur_normal;
			}
		}
	}

	return coll.entity;
}

bool World::Intersection(const Entity &e, std::vector<WorldCollision> &col) {
	AABB box = e.Bounds();
	Chunk::Pos reference = e.ChunkCoords();
	glm::mat4 M = e.Transform(reference);
	bool any = false;
	for (Chunk &cur_chunk : chunks) {
		if (manhattan_radius(cur_chunk.Position() - e.ChunkCoords()) > 1) {
			// chunk is not one of the 3x3x3 surrounding the entity
			// since there's no entity which can extent over 16 blocks, they can be skipped
			continue;
		}
		if (cur_chunk.Intersection(box, M, cur_chunk.Transform(reference), col)) {
			any = true;
		}
	}
	return any;
}


namespace {

std::vector<WorldCollision> col;

}

void World::Update(int dt) {
	for (Entity &entity : entities) {
		entity.Update(dt);
	}
	for (Entity &entity : entities) {
		col.clear();
		if (entity.WorldCollidable() && Intersection(entity, col)) {
			// entity collides with the world
			Resolve(entity, col);
		}
	}
	for (Player &player : players) {
		player.chunks->Rebase(player.entity->ChunkCoords());
	}
	for (auto iter = entities.begin(), end = entities.end(); iter != end;) {
		if (iter->CanRemove()) {
			iter = RemoveEntity(iter);
		} else {
			++iter;
		}
	}
}

void World::Resolve(Entity &e, std::vector<WorldCollision> &col) {
	// determine displacement for each cardinal axis and move entity accordingly
	glm::vec3 min_disp(0.0f);
	glm::vec3 max_disp(0.0f);
	for (const WorldCollision &c : col) {
		if (!c.Blocks()) continue;
		glm::vec3 local_disp(c.normal * c.depth);
		// swap if neccessary (normal may point away from the entity)
		if (dot(c.normal, e.Position() - c.BlockCoords()) < 0) {
			local_disp *= -1;
		}
		min_disp = min(min_disp, local_disp);
		max_disp = max(max_disp, local_disp);
	}
	// for each axis
	// if only one direction is set, use that as the final
	// if both directions are set, use average
	glm::vec3 final_disp(0.0f);
	for (int axis = 0; axis < 3; ++axis) {
		if (std::abs(min_disp[axis]) > std::numeric_limits<float>::epsilon()) {
			if (std::abs(max_disp[axis]) > std::numeric_limits<float>::epsilon()) {
				final_disp[axis] = (min_disp[axis] + max_disp[axis]) * 0.5f;
			} else {
				final_disp[axis] = min_disp[axis];
			}
		} else if (std::abs(max_disp[axis]) > std::numeric_limits<float>::epsilon()) {
			final_disp[axis] = max_disp[axis];
		}
	}
	e.Move(final_disp);
}

World::EntityHandle World::RemoveEntity(EntityHandle &eh) {
	// check for player
	for (auto player = players.begin(), end = players.end(); player != end;) {
		if (player->entity == &*eh) {
			chunks.UnregisterIndex(*player->chunks);
			player = players.erase(player);
			end = players.end();
		} else {
			++player;
		}
	}
	return entities.erase(eh);
}


void World::Render(Viewport &viewport) {
	DirectionalLighting &entity_prog = viewport.EntityProgram();
	entity_prog.SetLightDirection(light_direction);
	entity_prog.SetFogDensity(fog_density);

	for (Entity &entity : entities) {
		entity.Render(entity.ChunkTransform(players[0].entity->ChunkCoords()), entity_prog);
	}
}

}
