#ifndef BLANK_GEOMETRY_ROTATION_HPP_
#define BLANK_GEOMETRY_ROTATION_HPP_

#include <glm/glm.hpp>


namespace blank {

glm::mat3 find_rotation(const glm::vec3 &a, const glm::vec3 &b) noexcept;

}

#endif
