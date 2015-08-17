#ifndef BLANK_MODEL_GEOMETRY_H_
#define BLANK_MODEL_GEOMETRY_H_

#include <algorithm>
#include <glm/glm.hpp>


namespace blank {

constexpr float PI = 3.141592653589793238462643383279502884;
constexpr float PI_0p25 = PI * 0.25f;
constexpr float PI_0p5 = PI * 0.5f;
constexpr float PI_1p5 = PI * 1.5f;
constexpr float PI_2p0 = PI * 2.0f;

constexpr float DEG_RAD_FACTOR = PI / 180.0f;
constexpr float RAD_DEG_FACTOR = 180.0f / PI;

constexpr float deg2rad(float d) {
	return d * DEG_RAD_FACTOR;
}

constexpr float rad2deg(float r) {
	return r * RAD_DEG_FACTOR;
}


template<class T>
T manhattan_distance(const glm::tvec3<T> &a, const glm::tvec3<T> &b) {
	glm::tvec3<T> diff(abs(a - b));
	return diff.x + diff.y + diff.z;
}

template<class T>
T manhattan_radius(const glm::tvec3<T> &v) {
	glm::tvec3<T> a(abs(v));
	return std::max(a.x, std::max(a.y, a.z));
}


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
