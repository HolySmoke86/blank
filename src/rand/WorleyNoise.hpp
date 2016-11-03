#ifndef BLANK_RAND_WORLEYNOISE_HPP_
#define BLANK_RAND_WORLEYNOISE_HPP_

#include "../graphics/glm.hpp"


namespace blank {

/// implementation of Worley noise (aka Cell or Voroni noise)
class WorleyNoise {

public:
	explicit WorleyNoise(unsigned int seed) noexcept;

	float operator ()(const glm::vec3 &) const noexcept;

private:
	const unsigned int seed;
	const int num_points;

};

}

#endif
