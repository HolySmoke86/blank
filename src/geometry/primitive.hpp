#ifndef BLANK_GEOMETRY_PRIMITIVE_HPP_
#define BLANK_GEOMETRY_PRIMITIVE_HPP_

#include <algorithm>
#include <glm/glm.hpp>


namespace blank {

struct AABB {
	glm::vec3 min;
	glm::vec3 max;

	void Adjust() noexcept {
		if (max.x < min.x) std::swap(max.x, min.x);
		if (max.y < min.y) std::swap(max.y, min.y);
		if (max.z < min.z) std::swap(max.z, min.z);
	}

	glm::vec3 Center() const noexcept {
		return min + (max - min) * 0.5f;
	}

	/// return distance between origin and farthest vertex
	float OriginRadius() const noexcept {
		glm::vec3 high(glm::max(abs(min), abs(max)));
		return length(high);
	}
};

struct Ray {
	glm::vec3 orig;
	glm::vec3 dir;
};

bool Intersection(
	const Ray &,
	const AABB &,
	const glm::mat4 &M,
	float *dist = nullptr,
	glm::vec3 *normal = nullptr) noexcept;

bool Intersection(
	const AABB &a_box,
	const glm::mat4 &a_m,
	const AABB &b_box,
	const glm::mat4 &b_m,
	float &depth,
	glm::vec3 &normal) noexcept;

bool CullTest(const AABB &box, const glm::mat4 &MVP) noexcept;

}

#endif
