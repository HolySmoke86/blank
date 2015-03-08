#ifndef BLANK_NOISE_HPP_
#define BLANK_NOISE_HPP_

#include <glm/glm.hpp>


namespace blank {

/// (3D only) adaptation of Stefan Gustavson's SimplexNoise java class
class SimplexNoise {

public:
	explicit SimplexNoise(unsigned int seed);

	float operator ()(const glm::vec3 &) const;

private:
	unsigned char Perm(size_t idx) const;
	const glm::vec3 &Grad(size_t idx) const;

private:
	unsigned char perm[256];
	glm::vec3 grad[12];

};

}

#endif
