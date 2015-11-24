#ifndef BLANK_GEOMETRY_DISTANCE_HPP_
#define BLANK_GEOMETRY_DISTANCE_HPP_

#include <algorithm>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/norm.hpp>


namespace blank {

template <class T>
inline bool iszero(const T &v) noexcept {
	return length2(v) < std::numeric_limits<typename T::value_type>::epsilon();
}

template<class Vec>
inline void limit(Vec &v, float max) noexcept {
	float len2 = length2(v);
	float max2 = max * max;
	if (len2 > max2) {
		v = normalize(v) * max;
	}
}

template<class T>
T manhattan_distance(const glm::tvec3<T> &a, const glm::tvec3<T> &b) noexcept {
	return compAdd(abs(a - b));
}

template<class T>
T manhattan_radius(const glm::tvec3<T> &v) noexcept {
	return compMax(abs(v));
}

}

#endif
