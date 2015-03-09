#ifndef BLANK_GEOMETRY_H_
#define BLANK_GEOMETRY_H_

#include <algorithm>
#include <glm/glm.hpp>


namespace blank {

constexpr float PI = 3.141592653589793238462643383279502884;

struct AABB {
	glm::vec3 min;
	glm::vec3 max;

	void Adjust() {
		if (max.x < min.x) std::swap(max.x, min.x);
		if (max.y < min.y) std::swap(max.y, min.y);
		if (max.z < min.z) std::swap(max.z, min.z);
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
	glm::vec3 *normal = nullptr);

bool CullTest(const AABB &box, const glm::mat4 &MVP);

}

#endif
