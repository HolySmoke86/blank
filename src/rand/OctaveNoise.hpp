#ifndef BLANK_RAND_OCTAVENOISE_HPP_
#define BLANK_RAND_OCTAVENOISE_HPP_

#include <glm/glm.hpp>


namespace blank {

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
