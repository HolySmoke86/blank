#ifndef BLANK_WORLD_ENTITYCOLLISION_HPP_
#define BLANK_WORLD_ENTITYCOLLISION_HPP_


namespace blank {

class Entity;

struct EntityCollision {

	float depth;
	glm::vec3 normal;

	EntityCollision()
	: depth(0.0f), normal(0.0f), entity(nullptr) { }
	EntityCollision(Entity *e, float d, const glm::vec3 &n);
	~EntityCollision();

	EntityCollision(const EntityCollision &);
	EntityCollision &operator =(const EntityCollision &);

	/// check if an actual collision
	operator bool() const noexcept { return entity; }

	Entity &GetEntity() noexcept { return *entity; }
	const Entity &GetEntity() const noexcept { return *entity; }

private:
	Entity *entity;

};

}

#endif
