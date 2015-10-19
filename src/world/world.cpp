#include "Entity.hpp"
#include "EntityDerivative.hpp"
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
#include <iostream>
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
, tgt_vel(0.0f)
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

glm::mat4 Entity::Transform(const glm::ivec3 &reference) const noexcept {
	return state.Transform(reference);
}

glm::mat4 Entity::ViewTransform(const glm::ivec3 &reference) const noexcept {
	glm::mat4 transform = Transform(reference);
	if (model) {
		transform *= model.EyesTransform();
	}
	return transform;
}

Ray Entity::Aim(const Chunk::Pos &chunk_offset) const noexcept {
	glm::mat4 transform = ViewTransform(chunk_offset);
	glm::vec4 from = transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	from /= from.w;
	glm::vec4 to = transform * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	to /= to.w;
	return Ray{ glm::vec3(from), glm::normalize(glm::vec3(to - from)) };
}


EntityState::EntityState()
: chunk_pos(0)
, block_pos(0.0f)
, velocity(0.0f)
, orient(1.0f, 0.0f, 0.0f, 0.0f)
, ang_vel(0.0f) {

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

bool Player::SuitableSpawn(BlockLookup &spawn_block) const noexcept {
	if (!spawn_block || spawn_block.GetType().collide_block) {
		return false;
	}

	BlockLookup head_block(spawn_block.Next(Block::FACE_UP));
	if (!head_block || head_block.GetType().collide_block) {
		return false;
	}

	return true;
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

bool World::Intersection(const Entity &e, const EntityState &s, std::vector<WorldCollision> &col) {
	AABB box = e.Bounds();
	Chunk::Pos reference = s.chunk_pos;
	glm::mat4 M = s.Transform(reference);
	bool any = false;
	for (Chunk &cur_chunk : chunks) {
		if (manhattan_radius(cur_chunk.Position() - reference) > 1) {
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


void World::Update(int dt) {
	float fdt(dt * 0.001f);
	for (Entity &entity : entities) {
		Update(entity, fdt);
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

void World::Update(Entity &entity, float dt) {
	EntityState state(entity.GetState());

	EntityDerivative a(CalculateStep(entity, state, 0.0f, EntityDerivative()));
	EntityDerivative b(CalculateStep(entity, state, dt * 0.5f, a));
	EntityDerivative c(CalculateStep(entity, state, dt * 0.5f, b));
	EntityDerivative d(CalculateStep(entity, state, dt, c));

	EntityDerivative f;
	constexpr float sixth = 1.0f / 6.0f;
	f.position = sixth * ((a.position + 2.0f * (b.position + c.position)) + d.position);
	f.velocity = sixth * ((a.velocity + 2.0f * (b.velocity + c.velocity)) + d.velocity);
	f.orient = sixth * ((a.orient + 2.0f * (b.orient + c.orient)) + d.orient);

	state.block_pos += f.position * dt;
	state.velocity += f.velocity * dt;
	state.orient = delta_rot(f.orient, dt) * state.orient;
	state.AdjustPosition();

	entity.SetState(state);
}

EntityDerivative World::CalculateStep(
	const Entity &entity,
	const EntityState &cur,
	float dt,
	const EntityDerivative &delta
) {
	EntityState next(cur);
	next.block_pos += delta.position * dt;
	next.velocity += delta.velocity * dt;
	next.orient = delta_rot(cur.ang_vel, dt) * cur.orient;
	next.AdjustPosition();

	EntityDerivative out;
	out.position = next.velocity;
	out.velocity = CalculateForce(entity, next); // by mass = 1kg
	return out;
}

glm::vec3 World::CalculateForce(
	const Entity &entity,
	const EntityState &state
) {
	return ControlForce(entity, state) + CollisionForce(entity, state) + Gravity(entity, state);
}

glm::vec3 World::ControlForce(
	const Entity &entity,
	const EntityState &state
) {
	constexpr float k = 10.0f; // spring constant
	constexpr float b = 10.0f; // damper constant
	const glm::vec3 x(-entity.TargetVelocity()); // endpoint displacement from equilibrium, by 1s, in m
	const glm::vec3 v(state.velocity); // relative velocity between endpoints in m/s
	return ((-k) * x) - (b * v); // times 1kg/s, in kg*m/s²
}

namespace {

std::vector<WorldCollision> col;

}

glm::vec3 World::CollisionForce(
	const Entity &entity,
	const EntityState &state
) {
	col.clear();
	if (entity.WorldCollidable() && Intersection(entity, state, col)) {
		// determine displacement for each cardinal axis and move entity accordingly
		glm::vec3 min_pen(0.0f);
		glm::vec3 max_pen(0.0f);
		for (const WorldCollision &c : col) {
			if (!c.Blocks()) continue;
			glm::vec3 local_pen(c.normal * c.depth);
			// swap if neccessary (normal may point away from the entity)
			if (dot(c.normal, state.RelativePosition(c.ChunkPos()) - c.BlockCoords()) > 0) {
				local_pen *= -1;
			}
			min_pen = min(min_pen, local_pen);
			max_pen = max(max_pen, local_pen);
		}
		glm::vec3 penetration(min_pen + max_pen);
		glm::vec3 normal(normalize(penetration) * -1.0f);
		glm::vec3 normal_velocity(normal * dot(state.velocity, normal));
		// apply force proportional to penetration
		// use velocity projected onto normal as damper
		constexpr float k = 1000.0f; // spring constant
		constexpr float b = 100.0f; // damper constant
		const glm::vec3 x(penetration); // endpoint displacement from equilibrium in m
		const glm::vec3 v(normal_velocity); // relative velocity between endpoints in m/s
		return (((-k) * x) - (b * v)); // times 1kg/s, in kg*m/s²
	} else {
		return glm::vec3(0.0f);
	}
}

glm::vec3 World::Gravity(
	const Entity &entity,
	const EntityState &state
) {
	return glm::vec3(0.0f);
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
