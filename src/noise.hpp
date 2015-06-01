#ifndef BLANK_NOISE_HPP_
#define BLANK_NOISE_HPP_

#include <cstdint>
#include <limits>
#include <glm/glm.hpp>


namespace blank {

class GaloisLFSR {

public:
	// seed should be non-zero
	explicit GaloisLFSR(std::uint64_t seed) noexcept;

	// get the next bit
	bool operator ()() noexcept;

	template<class T>
	T operator ()(T &out) noexcept {
		constexpr int num_bits =
			std::numeric_limits<T>::digits +
			std::numeric_limits<T>::is_signed;
		for (int i = 0; i < num_bits; ++i) {
			operator ()();
		}
		return out = static_cast<T>(state);
	}

private:
	std::uint64_t state;
	// bits 64, 63, 61, and 60 set to 1 (counting from 1 lo to hi)
	static constexpr std::uint64_t mask = 0xD800000000000000;

};


/// (3D only) adaptation of Stefan Gustavson's SimplexNoise java class
class SimplexNoise {

public:
	explicit SimplexNoise(unsigned int seed) noexcept;

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


/// implementation of Worley noise (aka Cell or Voroni noise)
class WorleyNoise {

public:
	explicit WorleyNoise(unsigned int seed) noexcept;

	float operator ()(const glm::vec3 &) const noexcept;

private:
	const unsigned int seed;
	const int num_points;

};


template<class Noise>
float OctaveNoise(
	const Noise &noise,
	const glm::vec3 &in,
	int num,
	float persistence,
	float frequency = 1.0f,
	float amplitude = 1.0f,
	float growth = 2.0f
) {
	float total = 0.0f;
	float max = 0.0f;
	for (int i = 0; i < num; ++i) {
		total += noise(in * frequency) * amplitude;
		max += amplitude;
		amplitude *= persistence;
		frequency *= growth;
	}

	return total / max;
}

}

#endif
