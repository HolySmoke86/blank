#include "Generator.hpp"

#include "BlockType.hpp"
#include "BlockTypeRegistry.hpp"
#include "Chunk.hpp"
#include "../rand/OctaveNoise.hpp"

#include <glm/glm.hpp>


namespace blank {

namespace {

struct Candidate {
	const BlockType *type;
	float threshold;
	Candidate(const BlockType *type, float threshold)
	: type(type), threshold(threshold) { }
};

std::vector<Candidate> candidates;

}

Generator::Generator(const Config &config) noexcept
: config(config)
, types()
, min_solidity(2.0f)
, solidity_noise(config.seed ^ config.solidity.seed_mask)
, humidity_noise(config.seed ^ config.humidity.seed_mask)
, temperature_noise(config.seed ^ config.temperature.seed_mask)
, richness_noise(config.seed ^ config.richness.seed_mask)
, random_noise(config.seed ^ config.randomness.seed_mask) {

}

void Generator::LoadTypes(const BlockTypeRegistry &reg) {
	types.clear();
	min_solidity = 2.0f;
	for (const BlockType &type : reg) {
		if (type.generate) {
			types.push_back(&type);
			if (type.min_solidity < min_solidity) {
				min_solidity = type.min_solidity;
			}
		}
	}
	candidates.reserve(types.size());
}

void Generator::operator ()(Chunk &chunk) const noexcept {
	Chunk::Pos pos(chunk.Position());
	glm::vec3 coords(pos * Chunk::Extent());
	for (int z = 0; z < Chunk::depth; ++z) {
		for (int y = 0; y < Chunk::height; ++y) {
			for (int x = 0; x < Chunk::width; ++x) {
				Block::Pos block_pos(x, y, z);
				chunk.SetBlock(block_pos, Generate(coords + block_pos));
			}
		}
	}
	chunk.SetGenerated();
}

Block Generator::Generate(const glm::vec3 &pos) const noexcept {
	float solidity = GetValue(solidity_noise, pos, config.solidity);
	if (solidity < min_solidity) {
		return Block(0);
	}
	float humidity = GetValue(humidity_noise, pos, config.humidity);
	float temperature = GetValue(temperature_noise, pos, config.temperature);
	float richness = GetValue(richness_noise, pos, config.richness);

	candidates.clear();
	float total = 0.0f;
	for (const BlockType *type : types) {
		if (solidity < type->min_solidity || solidity > type->max_solidity) continue;
		if (humidity < type->min_humidity || humidity > type->max_humidity) continue;
		if (temperature < type->min_temperature || temperature > type->max_temperature) continue;
		if (richness < type->min_richness || richness > type->max_richness) continue;
		float solidity_match = 4.0f - ((solidity - type->mid_solidity) * (solidity - type->mid_solidity));
		float humidity_match = 4.0f - ((humidity - type->mid_humidity) * (humidity - type->mid_humidity));
		float temperature_match = 4.0f - ((temperature - type->mid_temperature) * (temperature - type->mid_temperature));
		float richness_match = 4.0f - ((richness - type->mid_richness) * (richness - type->mid_richness));
		float chance = (solidity_match + humidity_match + temperature_match + richness_match) * type->commonness;
		total += chance;
		candidates.emplace_back(type, total);
	}
	if (candidates.empty()) {
		return Block(0);
	}
	float random = GetValue(random_noise, pos, config.randomness);
	if (random < 0.0f) random += 1.0f;
	float value = random * total;
	// TODO: change to binary search
	for (const Candidate &cand : candidates) {
		if (value < cand.threshold) {
			return Block(cand.type->id);
		}
	}
	// theoretically, this should never happen
	return Block(candidates.back().type->id);
}

float Generator::GetValue(
	const SimplexNoise &noise,
	const glm::vec3 &pos,
	const Config::NoiseParam &conf
) noexcept {
	return OctaveNoise(
		noise,
		pos,
		conf.octaves,
		conf.persistence,
		conf.frequency,
		conf.amplitude,
		conf.growth
	);
}

}
