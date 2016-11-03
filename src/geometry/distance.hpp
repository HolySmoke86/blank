#ifndef BLANK_GEOMETRY_DISTANCE_HPP_
#define BLANK_GEOMETRY_DISTANCE_HPP_

#include "../graphics/glm.hpp"

#include <algorithm>
#include <limits>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/norm.hpp>


namespace blank {

template <class T>
inline bool iszero(const T &v) noexcept {
	return glm::length2(v) < std::numeric_limits<typename T::value_type>::epsilon();
}

template<class Vec>
inline void limit(Vec &v, float max) noexcept {
	float len2 = glm::length2(v);
	float max2 = max * max;
	if (len2 > max2) {
		v = glm::normalize(v) * max;
	}
}

template<class T, glm::precision P = glm::precision(0)>
T manhattan_distance(const TVEC3<T, P> &a, const TVEC3<T, P> &b) noexcept {
	return glm::compAdd(glm::abs(a - b));
}

template<class T, glm::precision P = glm::precision(0)>
T manhattan_radius(const TVEC3<T, P> &v) noexcept {
	return glm::compMax(glm::abs(v));
}

}

#endif
