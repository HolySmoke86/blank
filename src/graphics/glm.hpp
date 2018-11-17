#ifndef BLANK_GRAPHICS_GLM_HPP_
#define BLANK_GRAPHICS_GLM_HPP_

#ifndef GLM_FORCE_RADIANS
#  define GLM_FORCE_RADIANS 1
#endif

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>

// GLM moved tvec[1234] from glm::detail to glm in 0.9.6

#if GLM_VERSION < 96
#  define TVEC1 glm::detail::tvec1
#  define TVEC2 glm::detail::tvec2
#  define TVEC3 glm::detail::tvec3
#  define TVEC4 glm::detail::tvec4
#else
#  define TVEC1 glm::tvec1
#  define TVEC2 glm::tvec2
#  define TVEC3 glm::tvec3
#  define TVEC4 glm::tvec4
#endif

#endif
