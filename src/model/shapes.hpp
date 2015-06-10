#ifndef BLANK_MODEL_SHAPES_HPP_
#define BLANK_MODEL_SHAPES_HPP_

#include "geometry.hpp"
#include "Shape.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

class NullShape
: public Shape {

public:
	NullShape();

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const noexcept override;

};


class CuboidShape
: public Shape {

public:
	CuboidShape(const AABB &bounds);

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const noexcept override;

private:
	AABB bb;

};


class StairShape
: public Shape {

public:
	StairShape(const AABB &bounds, const glm::vec2 &clip);

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const noexcept override;

private:
	AABB top, bot;

};

}

#endif
