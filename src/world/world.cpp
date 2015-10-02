#include "Entity.hpp"
#include "EntityState.hpp"
#include "Player.hpp"
#include "World.hpp"

#include "ChunkIndex.hpp"
#include "EntityCollision.hpp"
#include "WorldCollision.hpp"
#include "../app/Assets.hpp"
#include "../graphics/Format.hpp"
#include "../graphics/Viewport.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <glm/gtx/io.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

Entity::Entity() noexcept
: model()
, id(-1)
, name("anonymous")
, bounds()
, state()
, ref_count(0)
, world_collision(false)
, dead(false) {

}


void Entity::Position(const glm::ivec3 &c, const glm::vec3 &b) noexcept {
	state.chunk_pos = c;
	state.block_pos = b;
}

void Entity::Position(const glm::vec3 &pos) noexcept {
	state.block_pos = pos;
	state.AdjustPosition();
}

Ray Entity::Aim(const Chunk::Pos &chunk_offset) const noexcept {
	glm::mat4 transform = Transform(chunk_offset);
	glm::vec4 from = transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	from /= from.w;
	glm::vec4 to = transform * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	to /= to.w;
	return Ray{ glm::vec3(from), glm::normalize(glm::vec3(to - from)) };
}

namespace {

glm::quat delta_rot(const glm::vec3 &av, float dt) {
	glm::vec3 half(av * dt * 0.5f);
	float mag = length(half);
	if (mag > 0.0f) {
		float smag = std::sin(mag) / mag;
		return glm::quat(std::cos(mag), half * smag);
	} else {
		return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	}
}

}

void Entity::Update(int dt) noexcept {
	state.Update(dt);
}


EntityState::EntityState()
: chunk_pos(0)
, block_pos(0.0f)
, velocity(0.0f)
, orient(1.0f, 0.0f, 0.0f, 0.0f)
, ang_vel(0.0f) {

}

void EntityState::Update(int dt) noexcept {
	float fdt = float(dt);
	block_pos += velocity * fdt;
	orient = delta_rot(ang_vel, fdt) * orient;
	AdjustPosition();
}

void EntityState::AdjustPosition() noexcept {
	while (block_pos.x >= Chunk::width) {
		block_pos.x -= Chunk::width;
		++chunk_pos.x;
	}
	while (block_pos.x < 0) {
		block_pos.x += Chunk::width;
		--chunk_pos.x;
	}
	while (block_pos.y >= Chunk::height) {
		block_pos.y -= Chunk::height;
		++chunk_pos.y;
	}
	while (block_pos.y < 0) {
		block_pos.y += Chunk::height;
		--chunk_pos.y;
	}
	while (block_pos.z >= Chunk::depth) {
		block_pos.z -= Chunk::depth;
		++chunk_pos.z;
	}
	while (block_pos.z < 0) {
		block_pos.z += Chunk::depth;
		--chunk_pos.z;
	}
}

glm::mat4 EntityState::Transform(const glm::ivec3 &reference) const noexcept {
	const glm::vec3 translation = RelativePosition(reference);
	glm::mat4 transform(toMat4(orient));
	transform[3].x = translation.x;
	transform[3].y = translation.y;
	transform[3].z = translation.z;
	return transform;
}


Player::Player(Entity &e, ChunkIndex &c)
: entity(e)
, chunks(c)
, inv_slot(0) {

}

Player::~Player() {

}

void Player::Update(int dt) {
	chunks.Rebase(entity.ChunkCoords());
}


World::World(const BlockTypeRegistry &types, const Config &config)
: config(config)
, block_type(types)
, chunks(types)
, players()
, entities()
, light_direction(config.light_direction)
, fog_density(config.fog_density) {

}

World::~World() {

}


Player *World::AddPlayer(const std::string &name) {
	for (Player &p : players) {
		if (p.Name() == name) {
			return nullptr;
		}
	}
	Entity &entity = AddEntity();
	entity.Name(name);
	entity.Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	entity.WorldCollidable(true);
	ChunkIndex &index = chunks.MakeIndex(entity.ChunkCoords(), 6);
	players.emplace_back(entity, index);
	return &players.back();
}

Player *World::AddPlayer(const std::string &name, std::uint32_t id) {
	for (Player &p : players) {
		if (p.Name() == name) {
			return nullptr;
		}
	}
	Entity *entity = AddEntity(id);
	if (!entity) {
		return nullptr;
	}
	entity->Name(name);
	entity->Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	entity->WorldCollidable(true);
	ChunkIndex &index = chunks.MakeIndex(entity->ChunkCoords(), 6);
	players.emplace_back(*entity, index);
	return &players.back();
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

Entity &World::ForceAddEntity(std::uint32_t id) {
	if (entities.empty() || entities.back().ID() < id) {
		entities.emplace_back();
		entities.back().ID(id);
		return entities.back();
	}

	auto position = entities.begin();
	auto end = entities.end();
	while (position != end && position->ID() < id) {
		++position;
	}
	if (position != end && position->ID() == id) {
		return *position;
	}
	auto entity = entities.emplace(position);
	entity->ID(id);
	return *entity;
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
		player.Update(dt);
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
	e.Position(e.Position() + final_disp);
}

World::EntityHandle World::RemoveEntity(EntityHandle &eh) {
	// check for player
	for (auto player = players.begin(), end = players.end(); player != end;) {
		if (&player->GetEntity() == &*eh) {
			chunks.UnregisterIndex(player->GetChunks());
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
		entity.Render(entity.Transform(players.front().GetEntity().ChunkCoords()), entity_prog);
	}
}

}
