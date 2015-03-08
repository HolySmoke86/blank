#ifndef BLANK_ENTITY_HPP_
#define BLANK_ENTITY_HPP_

#include "geometry.hpp"

#include <glm/glm.hpp>


namespace blank {

class Entity {

public:
	Entity();

	const glm::vec3 &Velocity() const { return velocity; }
	void Velocity(const glm::vec3 &);

	const glm::vec3 &Position() const { return position; }
	void Position(const glm::vec3 &);
	void Move(const glm::vec3 &delta);

	const glm::mat4 &Rotation() const { return rotation; }
	void Rotation(const glm::mat4 &);

	const glm::mat4 &Transform() const;
	Ray Aim() const;

	void Update(int dt);

private:
	glm::vec3 velocity;
	glm::vec3 position;

	glm::mat4 rotation;

	mutable glm::mat4 transform;
	mutable bool dirty;

};

}

#endif
