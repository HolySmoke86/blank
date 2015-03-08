#ifndef BLANKL_GLMIO_HPP_
#define BLANKL_GLMIO_HPP_

#include <ostream>
#include <glm/glm.hpp>


namespace blank {

template<class T>
std::ostream &operator <<(std::ostream &out, const glm::tvec3<T> &v) {
	return out << '<' << v.x << ", " << v.y << ", " << v.z << '>';
}

template<class T>
std::ostream &operator <<(std::ostream &out, const glm::tvec4<T> &v) {
	return out << '<' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << '>';
}

template<class T>
std::ostream &operator <<(std::ostream &out, const glm::tmat4x4<T> &m) {
	return out << '[' << m[0] << ", " << m[1] << ", " << m[2] << ", " << m[3] << ']';
}

}

#endif
