#include "Entity.hpp"

#include "../model/Shape.hpp"

#include <cmath>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace {

blank::EntityModel::Buffer model_buffer;

}

namespace blank {

Entity::Entity() noexcept
: model()
, name("anonymous")
, bounds()
, velocity(0, 0, 0)
, chunk(0, 0, 0)
, angular_velocity(0.0f)
, ref_count(0)
, world_collision(false)
, dead(false) {

}


void Entity::Position(const Chunk::Pos &c, const glm::vec3 &pos) noexcept {
	chunk = c;
	model.Position(pos);
}

void Entity::Position(const glm::vec3 &pos) noexcept {
	glm::vec3 position(pos);
	while (position.x >= Chunk::width) {
		position.x -= Chunk::width;
		++chunk.x;
	}
	while (position.x < 0) {
		position.x += Chunk::width;
		--chunk.x;
	}
	while (position.y >= Chunk::height) {
		position.y -= Chunk::height;
		++chunk.y;
	}
	while (position.y < 0) {
		position.y += Chunk::height;
		--chunk.y;
	}
	while (position.z >= Chunk::depth) {
		position.z -= Chunk::depth;
		++chunk.z;
	}
	while (position.z < 0) {
		position.z += Chunk::depth;
		--chunk.z;
	}
	model.Position(position);
}

void Entity::Move(const glm::vec3 &delta) noexcept {
	Position(Position() + delta);
}

void Entity::Rotate(const glm::quat &delta) noexcept {
	Orientation(delta * Orientation());
}

glm::mat4 Entity::ChunkTransform(const Chunk::Pos &chunk_offset) const noexcept {
	const glm::vec3 translation = glm::vec3((chunk - chunk_offset) * Chunk::Extent());
	return glm::translate(translation);
}

glm::mat4 Entity::Transform(const Chunk::Pos &chunk_offset) const noexcept {
	const glm::vec3 translation = glm::vec3((chunk - chunk_offset) * Chunk::Extent()) + Position();
	glm::mat4 transform(toMat4(Orientation()));
	transform[3].x = translation.x;
	transform[3].y = translation.y;
	transform[3].z = translation.z;
	return transform;
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
	float fdt = float(dt);
	Move(velocity * fdt);
	Rotate(delta_rot(angular_velocity, fdt));
}

}
