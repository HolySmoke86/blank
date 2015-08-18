#ifndef BLANK_WORLD_ENTITYCOLLISION_HPP_
#define BLANK_WORLD_ENTITYCOLLISION_HPP_


namespace blank {

class Entity;

struct EntityCollision {

	Entity *entity;

	float depth;
	glm::vec3 normal;

	EntityCollision()
	: entity(nullptr), depth(0.0f), normal(0.0f) { }
	EntityCollision(Entity *e, float d, const glm::vec3 &n)
	: entity(e), depth(d), normal(n) { }

	/// check if an actual collision
	operator bool() const noexcept { return entity; }

};

}

#endif
