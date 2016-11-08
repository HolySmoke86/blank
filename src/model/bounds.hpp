#ifndef BLANK_MODEL_BOUNDS_HPP_
#define BLANK_MODEL_BOUNDS_HPP_

#include "CollisionBounds.hpp"
#include "../geometry/primitive.hpp"
#include "../graphics/glm.hpp"

#include <vector>


namespace blank {

class CuboidBounds
: public CollisionBounds {

public:
	CuboidBounds(const AABB &bounds);

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const noexcept override;
	bool Intersects(const glm::mat4 &, const AABB &, const glm::mat4 &, float &, glm::vec3 &) const noexcept override;

private:
	AABB bb;

};


class StairBounds
: public CollisionBounds {

public:
	StairBounds(const AABB &bounds, const glm::vec2 &clip);

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const noexcept override;
	bool Intersects(const glm::mat4 &, const AABB &, const glm::mat4 &, float &, glm::vec3 &) const noexcept override;

private:
	AABB top, bot;

};

}

#endif
