#include "Entity.hpp"
#include "EntityController.hpp"
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
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

Entity::Entity() noexcept
: ctrl(nullptr)
, model()
, id(-1)
, name("anonymous")
, bounds()
, state()
, heading(0.0f, 0.0f, -1.0f)
, max_vel(5.0f)
, max_force(25.0f)
, ref_count(0)
, world_collision(false)
, dead(false)
, owns_controller(false) {

}

Entity::~Entity() noexcept {
	UnsetController();
}

Entity::Entity(const Entity &other) noexcept
: ctrl(other.ctrl)
, model(other.model)
, id(-1)
, name(other.name)
, bounds(other.bounds)
, state(other.state)
, model_transform(1.0f)
, view_transform(1.0f)
, speed(0.0f)
, heading(0.0f, 0.0f, -1.0f)
, max_vel(other.max_vel)
, max_force(other.max_force)
, ref_count(0)
, world_collision(other.world_collision)
, dead(other.dead)
, owns_controller(false) {

}

void Entity::SetController(EntityController *c) noexcept {
	UnsetController();
	ctrl = c;
	owns_controller = true;
}

void Entity::SetController(EntityController &c) noexcept {
	UnsetController();
	ctrl = &c;
	owns_controller = false;
}

void Entity::UnsetController() noexcept {
	if (ctrl && owns_controller) {
		delete ctrl;
	}
	ctrl = nullptr;
}

glm::vec3 Entity::ControlForce(const EntityState &s) const noexcept {
	if (HasController()) {
		return GetController().ControlForce(*this, s);
	} else {
		return -s.velocity;
	}
}

void Entity::Position(const glm::ivec3 &c, const glm::vec3 &b) noexcept {
	state.chunk_pos = c;
	state.block_pos = b;
}

void Entity::Position(const glm::vec3 &pos) noexcept {
	state.block_pos = pos;
	state.AdjustPosition();
}

void Entity::TurnHead(float dp, float dy) noexcept {
	SetHead(state.pitch + dp, state.yaw + dy);
}

void Entity::SetHead(float p, float y) noexcept {
	state.pitch = p;
	state.yaw = y;
	// TODO: I feel like this could be delayed
	UpdateModel();
}

glm::mat4 Entity::Transform(const glm::ivec3 &reference) const noexcept {
	return glm::translate(glm::vec3((state.chunk_pos - reference) * Chunk::Extent())) * model_transform;
}

glm::mat4 Entity::ViewTransform(const glm::ivec3 &reference) const noexcept {
	return Transform(reference) * view_transform;
}

Ray Entity::Aim(const Chunk::Pos &chunk_offset) const noexcept {
	glm::mat4 transform = ViewTransform(chunk_offset);
	return Ray{ glm::vec3(transform[3]), -glm::vec3(transform[2]) };
}

void Entity::UpdateModel() noexcept {
	state.AdjustHeading();
	if (model) {
		Part::State &body_state = model.BodyState();
		Part::State &eyes_state = model.EyesState();
		if (&body_state != &eyes_state) {
			body_state.orientation = glm::quat(glm::vec3(0.0f, state.yaw, 0.0f));
			eyes_state.orientation = glm::quat(glm::vec3(state.pitch, 0.0f, 0.0f));
		} else {
			eyes_state.orientation = glm::quat(glm::vec3(state.pitch, state.yaw, 0.0f));
		}
	}
}

void Entity::Update(float dt) {
	UpdateTransforms();
	UpdateHeading();
	if (HasController()) {
		GetController().Update(*this, dt);
	}
}

void Entity::UpdateTransforms() noexcept {
	// model transform is the one given by current state
	model_transform = state.Transform(state.chunk_pos);
	// view transform is either the model's eyes transform or,
	// should the entity have no model, the pitch (yaw already is
	// in model transform)
	if (model) {
		view_transform = model.EyesTransform();
	} else {
		view_transform = glm::eulerAngleX(state.pitch);
	}
}

void Entity::UpdateHeading() noexcept {
	speed = length(Velocity());
	if (speed > std::numeric_limits<float>::epsilon()) {
		heading = Velocity() / speed;
	} else {
		speed = 0.0f;
		// use -Z (forward axis) of local view transform
		heading = -glm::vec3(view_transform[2]);
	}
}


EntityController::~EntityController() {

}

bool EntityController::MaxOutForce(
	glm::vec3 &out,
	const glm::vec3 &add,
	float max
) noexcept {
	if (iszero(add) || any(isnan(add))) {
		return false;
	}
	float current = iszero(out) ? 0.0f : length(out);
	float remain = max - current;
	if (remain <= 0.0f) {
		return true;
	}
	float additional = length(add);
	if (additional > remain) {
		out += normalize(add) * remain;
		return true;
	} else {
		out += add;
		return false;
	}
}


EntityState::EntityState()
: chunk_pos(0)
, block_pos(0.0f)
, velocity(0.0f)
, orient(1.0f, 0.0f, 0.0f, 0.0f)
, pitch(0.0f)
, yaw(0.0f) {

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

void EntityState::AdjustHeading() noexcept {
	glm::clamp(pitch, -PI_0p5, PI_0p5);
	while (yaw > PI) {
		yaw -= PI_2p0;
	}
	while (yaw < -PI) {
		yaw += PI_2p0;
	}
}

glm::mat4 EntityState::Transform(const glm::ivec3 &reference) const noexcept {
	const glm::vec3 translation = RelativePosition(reference);
	glm::mat4 transform(toMat4(orient));
	transform[3] = glm::vec4(translation, 1.0f);
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
	entity.Bounds({ { -0.4f, -0.9f, -0.4f }, { 0.4f, 0.9f, 0.4f } });
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
	entity->Bounds({ { -0.4f, -0.9f, -0.4f }, { 0.4f, 0.9f, 0.4f } });
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
	return Intersection(box, M, reference, col);
}

bool World::Intersection(
	const AABB &box,
	const glm::mat4 &M,
	const glm::ivec3 &reference,
	std::vector<WorldCollision> &col
) {
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
	for (Entity &entity : entities) {
		entity.Update(fdt);
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

	state.block_pos += f.position * dt;
	state.velocity += f.velocity * dt;
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
	next.AdjustPosition();

	if (dot(next.velocity, next.velocity) > entity.MaxVelocity() * entity.MaxVelocity()) {
		next.velocity = normalize(next.velocity) * entity.MaxVelocity();
	}

	EntityDerivative out;
	out.position = next.velocity;
	out.velocity = CalculateForce(entity, next); // by mass = 1kg
	return out;
}

glm::vec3 World::CalculateForce(
	const Entity &entity,
	const EntityState &state
) {
	glm::vec3 force(ControlForce(entity, state) + CollisionForce(entity, state) + Gravity(entity, state));
	if (dot(force, force) > entity.MaxControlForce() * entity.MaxControlForce()) {
		return normalize(force) * entity.MaxControlForce();
	} else {
		return force;
	}
}

glm::vec3 World::ControlForce(
	const Entity &entity,
	const EntityState &state
) {
	return entity.ControlForce(state);
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
		glm::vec3 correction(0.0f);
		// only apply correction for axes where penetration is only in one direction
		for (std::size_t i = 0; i < 3; ++i) {
			if (min_pen[i] < -std::numeric_limits<float>::epsilon()) {
				if (max_pen[i] < std::numeric_limits<float>::epsilon()) {
					correction[i] = -min_pen[i];
				}
			} else {
				correction[i] = -max_pen[i];
			}
		}
		// correction may be zero in which case normalize() returns NaNs
		if (dot(correction, correction) < std::numeric_limits<float>::epsilon()) {
			return glm::vec3(0.0f);
		}
		glm::vec3 normal(normalize(correction));
		glm::vec3 normal_velocity(normal * dot(state.velocity, normal));
		// apply force proportional to penetration
		// use velocity projected onto normal as damper
		constexpr float k = 1000.0f; // spring constant
		constexpr float b = 10.0f; // damper constant
		const glm::vec3 x(-correction); // endpoint displacement from equilibrium in m
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
