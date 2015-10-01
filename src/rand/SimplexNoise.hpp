#ifndef BLANK_RAND_SIMPLEXNOISE_HPP_
#define BLANK_RAND_SIMPLEXNOISE_HPP_

#include <cstdint>
#include <glm/glm.hpp>


namespace blank {

/// (3D only) adaptation of Stefan Gustavson's SimplexNoise java class
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

};

}

#endif
