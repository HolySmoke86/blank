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
#include "../geometry/const.hpp"
#include "../geometry/distance.hpp"
#include "../graphics/Format.hpp"
#include "../graphics/Viewport.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

Entity::Entity() noexcept
: ctrl(nullptr)
, model()
, id(-1)
, name("anonymous")
, bounds()
, radius(0.0f)
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
	state.pos.chunk = c;
	state.pos.block = b;
}

void Entity::Position(const glm::vec3 &pos) noexcept {
	state.pos.block = pos;
	state.AdjustPosition();
}

void Entity::TurnHead(float dp, float dy) noexcept {
	SetHead(state.pitch + dp, state.yaw + dy);
}

void Entity::SetHead(float p, float y) noexcept {
	state.pitch = p;
	state.yaw = y;
}

glm::mat4 Entity::Transform(const glm::ivec3 &reference) const noexcept {
	return glm::translate(glm::vec3((state.pos.chunk - reference) * ExactLocation::Extent())) * model_transform;
}

glm::mat4 Entity::ViewTransform(const glm::ivec3 &reference) const noexcept {
	return Transform(reference) * view_transform;
}

Ray Entity::Aim(const ExactLocation::Coarse &chunk_offset) const noexcept {
	glm::mat4 transform = ViewTransform(chunk_offset);
	return Ray{ glm::vec3(transform[3]), -glm::vec3(transform[2]) };
}

void Entity::Update(float dt) {
	UpdateTransforms();
	UpdateHeading();
	if (HasController()) {
		GetController().Update(*this, dt);
	}
	UpdateModel(dt);
}

void Entity::UpdateTransforms() noexcept {
	// model transform is the one given by current state
	model_transform = state.Transform(state.pos.chunk);
	// view transform is either the model's eyes transform or,
	// should the entity have no model, the pitch (yaw already is
	// in model transform)
	if (model) {
		view_transform = model.EyesTransform();
	} else {
		view_transform = toMat4(glm::quat(glm::vec3(state.pitch, state.yaw, 0.0f)));
	}
}

void Entity::UpdateHeading() noexcept {
	speed = length(Velocity());
	if (speed > std::numeric_limits<float>::epsilon()) {
		heading = Velocity() / speed;
	} else {
		speed = 0.0f;
		// use -Z (forward axis) of model transform (our "chest")
		heading = -glm::vec3(model_transform[2]);
	}
}

void Entity::UpdateModel(float dt) noexcept {
	// first, sanitize the pitch and yaw fields of state (our input)
	// those indicate the head orientation in the entity's local cosystem
	state.AdjustHeading();
	// TODO: this flickers horrible and also shouldn't be based on velocity, but on control force
	//OrientBody(dt);
	OrientHead(dt);
}

void Entity::OrientBody(float dt) noexcept {
	// maximum body rotation per second (due to velocity orientation) (90°)
	constexpr float max_body_turn_per_second = PI_0p5;
	const float max_body_turn = max_body_turn_per_second * dt;
	// minimum speed to apply body correction
	constexpr float min_speed = 0.0625f;
	// use local Y as up
	const glm::vec3 up(model_transform[1]);
	if (speed > min_speed) {
		// check if our orientation and velocity are aligned
		const glm::vec3 forward(-model_transform[2]);
		// facing is local -Z rotated about local Y by yaw and transformed into world space
		const glm::vec3 facing(normalize(glm::vec3(glm::vec4(rotateY(glm::vec3(0.0f, 0.0f, -1.0f), state.yaw), 0.0f) * transpose(model_transform))));
		// only adjust if velocity isn't almost parallel to up
		float vel_dot_up = dot(Velocity(), up);
		if (std::abs(1.0f - std::abs(vel_dot_up)) > std::numeric_limits<float>::epsilon()) {
			// get direction of velocity projected onto model plane
			glm::vec3 direction(normalize(Velocity() - (Velocity() * vel_dot_up)));
			// if velocity points away from our facing (with a little bias), flip it around
			// (the entity is "walking backwards")
			if (dot(facing, direction) < -0.1f) {
				direction = -direction;
			}
			// calculate the difference between forward and direction
			const float absolute_difference = std::acos(dot(forward, direction));
			// if direction is clockwise with respect to up vector, invert the angle
			const float relative_difference = dot(cross(forward, direction), up) < 0.0f
				? -absolute_difference
				: absolute_difference;
			// only correct by half the difference max
			const float correction = glm::clamp(relative_difference * 0.5f, -max_body_turn, max_body_turn);
			if (ID() == 1) {
				std::cout << "orientation before: " << state.orient << std::endl;
				std::cout << "up:        " << up << std::endl;
				std::cout << "forward:   " << forward << std::endl;
				std::cout << "facing:    " << facing << std::endl;
				std::cout << "direction: " << direction << std::endl;
				std::cout << "difference: " << glm::degrees(relative_difference) << "°" << std::endl;
				std::cout << "correction: " << glm::degrees(correction) << "°" << std::endl;
				std::cout  << std::endl;
			}
			// now rotate body by correction and head by -correction
			state.orient = rotate(state.orient, correction, up);
			state.yaw -= correction;
		}
	}
}

void Entity::OrientHead(float dt) noexcept {
	// maximum yaw of head (60°)
	constexpr float max_head_yaw = PI / 3.0f;
	// use local Y as up
	const glm::vec3 up(model_transform[1]);
	// if yaw is bigger than max, rotate the body to accomodate
	if (std::abs(state.yaw) > max_head_yaw) {
		float deviation = state.yaw < 0.0f ? state.yaw + max_head_yaw : state.yaw - max_head_yaw;
		// rotate the entity by deviation about local Y
		state.orient = rotate(state.orient, deviation, up);
		// and remove from head yaw
		state.yaw -= deviation;
		// shouldn't be necessary if max_head_yaw is < PI, but just to be sure :p
		state.AdjustHeading();
	}
	// update model if any
	if (model) {
		model.EyesState().orientation = glm::quat(glm::vec3(state.pitch, state.yaw, 0.0f));
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
: pos()
, velocity(0.0f)
, orient(1.0f, 0.0f, 0.0f, 0.0f)
, pitch(0.0f)
, yaw(0.0f) {

}

void EntityState::AdjustPosition() noexcept {
	pos.Correct();
}

void EntityState::AdjustHeading() noexcept {
	pitch = glm::clamp(pitch, -PI_0p5, PI_0p5);
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
	for (Entity &e : entities) {
		e.Kill();
	}
	std::size_t removed = 0;
	do {
		removed = 0;
		for (auto e = entities.begin(), end = entities.end(); e != end; ++e) {
			if (e->CanRemove()) {
				e = RemoveEntity(e);
				end = entities.end();
				++removed;
			}
		}
	} while (removed > 0 && !entities.empty());
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
	const ExactLocation::Coarse &reference,
	WorldCollision &coll
) {
	candidates.clear();

	for (Chunk &cur_chunk : chunks) {
		float cur_dist;
		if (cur_chunk.Intersection(ray, reference, cur_dist)) {
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
		if (cand.chunk->Intersection(ray, reference, cur_coll)) {
			if (cur_coll.depth < coll.depth) {
				coll = cur_coll;
			}
		}
	}

	return coll.chunk;
}

bool World::Intersection(
	const Ray &ray,
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
		if (blank::Intersection(ray, cur_entity.Bounds(), cur_entity.Transform(reference.ChunkCoords()), &cur_dist, &cur_normal)) {
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
	// TODO: make special case for entities here and in Chunk::Intersection so entity's bounding radius
	//       doesn't have to be calculated over and over again (sqrt)
	glm::ivec3 reference = s.pos.chunk;
	glm::mat4 M = s.Transform(reference);

	ExactLocation::Coarse begin(reference - 1);
	ExactLocation::Coarse end(reference + 2);

	bool any = false;
	for (ExactLocation::Coarse pos(begin); pos.z < end.y; ++pos.z) {
		for (pos.y = begin.y; pos.y < end.y; ++pos.y) {
			for (pos.x = begin.x; pos.x < end.x; ++pos.x) {
				Chunk *chunk = chunks.Get(pos);
				if (chunk && chunk->Intersection(e, M, chunk->Transform(reference), col)) {
					any = true;
				}
			}
		}
	}
	return any;
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
	f.position = sixth * (a.position + 2.0f * (b.position + c.position) + d.position);
	f.velocity = sixth * (a.velocity + 2.0f * (b.velocity + c.velocity) + d.velocity);

	state.pos.block += f.position * dt;
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
	next.pos.block += delta.position * dt;
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
		glm::vec3 correction = -CombinedInterpenetration(state, col);
		// correction may be zero in which case normalize() returns NaNs
		if (iszero(correction)) {
			return glm::vec3(0.0f);
		}
		// if entity is already going in the direction of correction,
		// let the problem resolve itself
		if (dot(state.velocity, correction) >= 0.0f) {
			return glm::vec3(0.0f);
		}
		glm::vec3 normal_velocity(proj(state.velocity, correction));
		// apply force proportional to penetration
		// use velocity projected onto correction as damper
		constexpr float k = 1000.0f; // spring constant
		constexpr float b = 10.0f; // damper constant
		const glm::vec3 x(-correction); // endpoint displacement from equilibrium in m
		const glm::vec3 v(normal_velocity); // relative velocity between endpoints in m/s
		return (((-k) * x) - (b * v)); // times 1kg/s, in kg*m/s²
	} else {
		return glm::vec3(0.0f);
	}
}

glm::vec3 World::CombinedInterpenetration(
	const EntityState &state,
	const std::vector<WorldCollision> &col
) noexcept {
	// determine displacement for each cardinal axis and move entity accordingly
	glm::vec3 min_pen(0.0f);
	glm::vec3 max_pen(0.0f);
	for (const WorldCollision &c : col) {
		if (!c.Blocks()) continue;
		glm::vec3 normal(c.normal);
		// swap if neccessary (normal may point away from the entity)
		if (dot(normal, state.RelativePosition(c.ChunkPos()) - c.BlockCoords()) < 0) {
			normal = -normal;
		}
		// check if block surface is "inside"
		Block::Face coll_face = Block::NormalFace(normal);
		BlockLookup neighbor(c.chunk, c.BlockPos(), coll_face);
		if (neighbor && neighbor.FaceFilled(Block::Opposite(coll_face))) {
			// yep, so ignore this contact
			continue;
		}
		glm::vec3 local_pen(normal * c.depth);
		min_pen = min(min_pen, local_pen);
		max_pen = max(max_pen, local_pen);
	}
	glm::vec3 pen(0.0f);
	// only apply correction for axes where penetration is only in one direction
	for (std::size_t i = 0; i < 3; ++i) {
		if (min_pen[i] < -std::numeric_limits<float>::epsilon()) {
			if (max_pen[i] < std::numeric_limits<float>::epsilon()) {
				pen[i] = min_pen[i];
			}
		} else {
			pen[i] = max_pen[i];
		}
	}
	return pen;
}

glm::vec3 World::Gravity(
	const Entity &entity,
	const EntityState &state
) {
	glm::vec3 force(0.0f);
	ExactLocation::Coarse begin(state.pos.chunk - 1);
	ExactLocation::Coarse end(state.pos.chunk + 2);

	for (ExactLocation::Coarse pos(begin); pos.z < end.z; ++pos.z) {
		for (pos.y = begin.y; pos.y < end.y; ++pos.y) {
			for (pos.x = begin.x; pos.x < end.x; ++pos.x) {
				Chunk *chunk = chunks.Get(pos);
				if (chunk) {
					force += chunk->GravityAt(state.pos);
				}
			}
		}
	}

	return force;
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
	entity_prog.SetFogDensity(fog_density);

	glm::vec3 light_dir;
	glm::vec3 light_col;
	glm::vec3 ambient_col;
	for (Entity &entity : entities) {
		glm::mat4 M(entity.Transform(players.front().GetEntity().ChunkCoords()));
		if (!CullTest(entity.Bounds(), entity_prog.GetVP() * M)) {
			GetLight(entity, light_dir, light_col, ambient_col);
			entity_prog.SetLightDirection(light_dir);
			entity_prog.SetLightColor(light_col);
			entity_prog.SetAmbientColor(ambient_col);
			entity.Render(M, entity_prog);
		}
	}
}

// this should interpolate based on the fractional part of entity's block position
void World::GetLight(
	const Entity &e,
	glm::vec3 &dir,
	glm::vec3 &col,
	glm::vec3 &amb
) {
	Chunk *chunk = chunks.Get(e.ChunkCoords());
	if (!chunk) {
		// chunk unavailable, so make it really dark and from
		// some arbitrary direction
		dir = glm::vec3(1.0f, 2.0f, 3.0f);
		col = glm::vec3(0.025f); // ~0.8^15
		return;
	}
	glm::ivec3 base(e.Position());
	int base_light = chunk->GetLight(base);
	int max_light = 0;
	int min_light = 15;
	glm::ivec3 acc(0, 0, 0);
	for (glm::ivec3 offset(-1, -1, -1); offset.z < 2; ++offset.z) {
		for (offset.y = -1; offset.y < 2; ++offset.y) {
			for (offset.x = -1; offset.x < 2; ++offset.x) {
				BlockLookup block(chunk, base + offset);
				if (!block) {
					// missing, just ignore it
					continue;
				}
				// otherwise, accumulate the difference times direction
				acc += offset * (base_light - block.GetLight());
				max_light = std::max(max_light, block.GetLight());
				min_light = std::min(min_light, block.GetLight());
			}
		}
	}
	dir = acc;
	col = glm::vec3(std::pow(0.8f, 15 - max_light));
	amb = glm::vec3(std::pow(0.8f, 15 - min_light));
}

namespace {

PrimitiveMesh::Buffer debug_buf;

}

void World::RenderDebug(Viewport &viewport) {
	PrimitiveMesh debug_mesh;
	PlainColor &prog = viewport.WorldColorProgram();
	for (const Entity &entity : entities) {
		debug_buf.OutlineBox(entity.Bounds(), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		debug_mesh.Update(debug_buf);
		prog.SetM(entity.Transform(players.front().GetEntity().ChunkCoords()));
		debug_mesh.DrawLines();
	}
}

}
