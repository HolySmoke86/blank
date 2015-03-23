#ifndef BLANK_NOISE_HPP_
#define BLANK_NOISE_HPP_

#include <cstdint>
#include <limits>
#include <glm/glm.hpp>


namespace blank {

class GaloisLFSR {

public:
	// seed should be non-zero
	explicit GaloisLFSR(std::uint64_t seed);

	// get the next bit
	bool operator ()();

	template<class T>
	void operator ()(T &out) {
		constexpr int num_bits =
			std::numeric_limits<T>::digits +
			std::numeric_limits<T>::is_signed;
		for (int i = 0; i < num_bits; ++i) {
			operator ()();
		}
		out = static_cast<T>(state);
	}

private:
	std::uint64_t state;
	// bits 64, 63, 61, and 60 set to 1 (counting from 1 lo to hi)
	static constexpr std::uint64_t mask = 0xD800000000000000;

};

/// (3D only) adaptation of Stefan Gustavson's SimplexNoise java class
class SimplexNoise {

public:
	explicit SimplexNoise(unsigned int seed);

	float operator ()(const glm::vec3 &) const;

private:
	unsigned char Perm(size_t idx) const;
	const glm::vec3 &Grad(size_t idx) const;

private:
	unsigned char perm[512];
	glm::vec3 grad[12];

};


/// implementation of Worley noise (aka Cell or Voroni noise)
class WorleyNoise {

public:
	explicit WorleyNoise(unsigned int seed);

	float operator ()(const glm::vec3 &) const;

private:
	const unsigned int seed;
	const int num_points;

};

}

#endif
