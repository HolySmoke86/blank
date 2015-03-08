#include "entity.hpp"

#include <glm/gtx/transform.hpp>


namespace blank {

Entity::Entity()
: velocity()
, position()
, rotation(1.0f)
, transform(1.0f)
, dirty(false) {

}


void Entity::Velocity(const glm::vec3 &vel) {
	velocity = vel;
}

void Entity::Position(const glm::vec3 &pos) {
	position = pos;
	dirty = true;
}

void Entity::Move(const glm::vec3 &delta) {
	position += delta;
	dirty = true;
}

void Entity::Rotation(const glm::mat4 &rot) {
	rotation = rot;
}

const glm::mat4 &Entity::Transform() const {
	if (dirty) {
		transform = glm::translate(position) * rotation;
		dirty = false;
	}
	return transform;
}

Ray Entity::Aim() const {
	Transform();
	glm::vec4 from = transform * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	from /= from.w;
	glm::vec4 to = transform * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	to /= to.w;
	return Ray{ glm::vec3(from), glm::normalize(glm::vec3(to - from)) };
}

void Entity::Update(int dt) {
	Move(velocity * float(dt));
}

}
