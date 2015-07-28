#ifndef BLANK_GRAPHICS_ALIGN_HPP_
#define BLANK_GRAPHICS_ALIGN_HPP_

#include <glm/glm.hpp>


namespace blank {

enum class Align {
	BEGIN,
	MIDDLE,
	END,
};

enum class Gravity {
	NORTH_WEST,
	NORTH,
	NORTH_EAST,
	WEST,
	CENTER,
	EAST,
	SOUTH_WEST,
	SOUTH,
	SOUTH_EAST,
};

inline Align get_x(Gravity g) noexcept {
	return Align(int(g) % 3);
}

inline Align get_y(Gravity g) noexcept {
	return Align(int(g) / 3);
}

inline Gravity get_gravity(Align x, Align y) noexcept {
	return Gravity(int(y) * 3 + int(x));
}

inline glm::vec2 align(
	Gravity g,
	const glm::vec2 &size,
	const glm::vec2 &offset = glm::vec2(0.0f, 0.0f)
) {
	return glm::vec2(
		size.x * 0.5 * int(get_x(g)) + offset.x,
		size.y * 0.5 * int(get_y(g)) + offset.y
	);
}

}

#endif
