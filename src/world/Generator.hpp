#ifndef BLANK_WORLD_GENERATOR_HPP_
#define BLANK_WORLD_GENERATOR_HPP_

#include "../rand/SimplexNoise.hpp"
#include "../rand/WorleyNoise.hpp"

#include <cstdint>
#include <glm/glm.hpp>


namespace blank {

class Block;
class BlockTypeRegistry;
class Chunk;

class Generator {

public:
	struct Config {
		std::uint64_t seed = 0;
		struct NoiseParam {
			std::uint64_t seed_mask;
			int octaves;
			float persistence;
			float frequency;
			float amplitude;
			float growth;
		};
		NoiseParam solidity = { 0xA85033F6BCBDD110, 3, 0.5f, 1.0f/64.0f, 2.0f, 2.0f };
		NoiseParam humidity = { 0x3A463FB24B04A901, 3, 0.5f, 1.0f/256.0f, 2.0f, 2.0f };
		NoiseParam temperature = { 0x2530BA6C6134A9FB, 3, 0.5f, 1.0f/512.0f, 2.0f, 2.0f };
		NoiseParam richness = { 0x95A179F180103446, 3, 0.5f, 1.0f/128.0f, 2.0f, 2.0f };
		NoiseParam randomness = { 0x074453EEE1496390, 3, 0.5f, 1.0f/16.0f, 2.0f, 2.0f };
	};

	explicit Generator(const Config &, const BlockTypeRegistry &) noexcept;

	// scan types for generation
	void Scan();

	void operator ()(Chunk &) const noexcept;

private:
	Block Generate(const glm::vec3 &position) const noexcept;
	static float GetValue(
		const SimplexNoise &,
		const glm::vec3 &,
		const Config::NoiseParam &) noexcept;

private:
	const Config &config;
	const BlockTypeRegistry &types;
	SimplexNoise solidity_noise;
	SimplexNoise humidity_noise;
	SimplexNoise temperature_noise;
	SimplexNoise richness_noise;
	SimplexNoise random_noise;

};

}

#endif
