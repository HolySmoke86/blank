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
: shape(nullptr)
, model()
, name("anonymous")
, bounds()
, velocity(0, 0, 0)
, position(0, 0, 0)
, chunk(0, 0, 0)
, angular_velocity(0.0f)
, rotation(1.0f, 0.0f, 0.0f, 0.0f)
, world_collision(false)
, remove(false) {

}


void Entity::SetShape(const Shape *s, const glm::vec3 &color) {
	shape = s;
	model_buffer.Clear();
	shape->Vertices(model_buffer.vertices, model_buffer.normals, model_buffer.indices);
	model_buffer.colors.resize(shape->VertexCount(), color);
	model.Update(model_buffer);
}

void Entity::SetShapeless() noexcept {
	shape = nullptr;
}


void Entity::Velocity(const glm::vec3 &vel) noexcept {
	velocity = vel;
}

void Entity::Position(const Chunk::Pos &c, const Block::Pos &pos) noexcept {
	chunk = c;
	position = pos;
}

void Entity::Position(const Block::Pos &pos) noexcept {
	position = pos;
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
}

void Entity::Move(const glm::vec3 &delta) noexcept {
	Position(position + delta);
}

void Entity::AngularVelocity(const glm::vec3 &v) noexcept {
	angular_velocity = v;
}

void Entity::Rotation(const glm::quat &rot) noexcept {
	rotation = rot;
}

void Entity::Rotate(const glm::quat &delta) noexcept {
	Rotation(delta * Rotation());
}

glm::mat4 Entity::Transform(const Chunk::Pos &chunk_offset) const noexcept {
	const glm::vec3 translation = glm::vec3((chunk - chunk_offset) * Chunk::Extent()) + position;
	glm::mat4 transform(toMat4(Rotation()));
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
