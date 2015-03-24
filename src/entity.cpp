#include "entity.hpp"

#include "geometry.hpp"
#include "shape.hpp"

#include <cmath>
#include <glm/gtx/transform.hpp>

namespace {

blank::Model::Buffer model_buffer;

}

namespace blank {

Entity::Entity()
: shape(nullptr)
, model()
, velocity(0, 0, 0)
, position(0, 0, 0)
, chunk(0, 0, 0)
, angular_velocity(1.0f, 0.0f, 0.0f, 0.0f)
, rotation(1.0f) {

}


void Entity::SetShape(const Shape *s, const glm::vec3 &color) {
	shape = s;
	model_buffer.Clear();
	shape->Vertices(model_buffer.vertices, model_buffer.normals, model_buffer.indices);
	model_buffer.colors.resize(shape->VertexCount(), color);
	model.Update(model_buffer);
}

void Entity::SetShapeless() {
	shape = nullptr;
}


void Entity::Velocity(const glm::vec3 &vel) {
	velocity = vel;
}

void Entity::Position(const Block::Pos &pos) {
	position = pos;
	while (position.x >= Chunk::Width()) {
		position.x -= Chunk::Width();
		++chunk.x;
	}
	while (position.x < 0) {
		position.x += Chunk::Width();
		--chunk.x;
	}
	while (position.y >= Chunk::Height()) {
		position.y -= Chunk::Height();
		++chunk.y;
	}
	while (position.y < 0) {
		position.y += Chunk::Height();
		--chunk.y;
	}
	while (position.z >= Chunk::Depth()) {
		position.z -= Chunk::Depth();
		++chunk.z;
	}
	while (position.z < 0) {
		position.z += Chunk::Depth();
		--chunk.z;
	}
}

void Entity::Move(const glm::vec3 &delta) {
	Position(position + delta);
}

void Entity::AngularVelocity(const glm::quat &v) {
	angular_velocity = v;
}

void Entity::Rotation(const glm::mat4 &rot) {
	rotation = rot;
}

void Entity::Rotate(const glm::quat &delta) {
	Rotation(rotation * glm::mat4_cast(delta));
}

glm::mat4 Entity::Transform(const Chunk::Pos &chunk_offset) const {
	const glm::vec3 chunk_pos = (chunk - chunk_offset) * Chunk::Extent();
	return glm::translate(position + chunk_pos) * rotation;
}

Ray Entity::Aim(const Chunk::Pos &chunk_offset) const {
	glm::mat4 transform = Transform(chunk_offset);
	glm::vec4 from = transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	from /= from.w;
	glm::vec4 to = transform * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	to /= to.w;
	return Ray{ glm::vec3(from), glm::normalize(glm::vec3(to - from)) };
}

void Entity::Update(int dt) {
	Move(velocity * float(dt));
	Rotate(angular_velocity * float(dt));
}


void Entity::Draw() {
	model.Draw();
}

}
