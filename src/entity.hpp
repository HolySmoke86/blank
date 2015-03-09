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

	const glm::tvec3<int> ChunkCoords() const { return chunk; }

	const glm::mat4 &Rotation() const { return rotation; }
	void Rotation(const glm::mat4 &);

	glm::mat4 Transform(const glm::tvec3<int> &chunk_offset) const;
	Ray Aim(const glm::tvec3<int> &chunk_offset) const;

	void Update(int dt);

private:
	glm::vec3 velocity;
	glm::vec3 position;
	glm::tvec3<int> chunk;

	glm::mat4 rotation;

};

}

#endif
