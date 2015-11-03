#ifndef BLANK_GEOMETRY_DISTANCE_HPP_
#define BLANK_GEOMETRY_DISTANCE_HPP_

#include <algorithm>
#include <limits>
#include <glm/glm.hpp>


namespace blank {

inline float length_squared(const glm::vec3 &v) noexcept {
	return dot(v, v);
}

inline float distance_squared(const glm::vec3 &a, const glm::vec3 &b) noexcept {
	return length_squared(a - b);
}

template <class T>
inline bool iszero(const T &v) noexcept {
	return length_squared(v) < std::numeric_limits<typename T::value_type>::epsilon();
}

template<class T>
T manhattan_distance(const glm::tvec3<T> &a, const glm::tvec3<T> &b) noexcept {
	glm::tvec3<T> diff(abs(a - b));
	return diff.x + diff.y + diff.z;
}

template<class T>
T manhattan_radius(const glm::tvec3<T> &v) noexcept {
	glm::tvec3<T> a(abs(v));
	return std::max(a.x, std::max(a.y, a.z));
}

}

#endif
