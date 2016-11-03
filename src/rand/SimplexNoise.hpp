#ifndef BLANK_RAND_SIMPLEXNOISE_HPP_
#define BLANK_RAND_SIMPLEXNOISE_HPP_

#include "../graphics/glm.hpp"

#include <cstdint>


namespace blank {

class SimplexNoise {

public:
	explicit SimplexNoise(std::uint64_t seed) noexcept;

	float operator ()(const glm::vec3 &) const noexcept;

private:
	int Perm(int idx) const noexcept;
	int Perm12(int idx) const noexcept;
	const glm::vec3 &Grad(int idx) const noexcept;

private:
	int perm[512];
	int perm12[512];
	glm::vec3 grad[12];
	glm::ivec3 second_ints[8];
	glm::ivec3 third_ints[8];
	glm::vec3 second_floats[8];
	glm::vec3 third_floats[8];

};

}

#endif
