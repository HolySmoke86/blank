#include "Generator.hpp"

#include "BlockType.hpp"
#include "BlockTypeRegistry.hpp"
#include "Chunk.hpp"
#include "../graphics/glm.hpp"
#include "../rand/OctaveNoise.hpp"


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
			if (type.solidity.Min() < min_solidity) {
				min_solidity = type.solidity.Min();
			}
		}
	}
	candidates.reserve(types.size());
}

namespace {

struct Interpolation {
	/// sample points for interpolation
	/// given coordinates should be the absoloute position of the chunk's (0,0,0) block
	Interpolation(
		const SimplexNoise &noise,
		const glm::vec3 &base,
		const Generator::Config::NoiseParam &conf
	) noexcept {
		for (int z = 0; z < 5; ++z) {
			for (int y = 0; y < 5; ++y) {
				for (int x = 0; x < 5; ++x) {
					samples[z][y][x] = OctaveNoise(
						noise,
						base + (glm::vec3(x, y, z) * 4.0f),
						conf.octaves,
						conf.persistence,
						conf.frequency,
						conf.amplitude,
						conf.growth
					);
				}
			}
		}
	}
	float samples[5][5][5];
};

struct Parameters {
	glm::ivec3 a;
	glm::ivec3 b;
	glm::ivec3 d;
};

struct Detail {
	float humidity;
	float temperature;
	float richness;
	float randomness;
};

}

struct Generator::ValueField {

	Interpolation solidity;
	Interpolation humidity;
	Interpolation temperature;
	Interpolation richness;
	Interpolation randomness;

	static Parameters GetParams(const glm::ivec3 &pos) noexcept {
		Parameters p;
		p.a = pos / 4;
		p.b = p.a + 1;
		p.d = pos % 4;
		return p;
	}

	static float Interpolate(const Interpolation &i, const Parameters &p) noexcept {
		constexpr float A[4] = { 1.0f, 0.75f, 0.5f, 0.25f };
		constexpr float B[4] = { 0.0f, 0.25f, 0.5f, 0.75f };
		const float l1[4] = {
			i.samples[p.a.z][p.a.y][p.a.x] * A[p.d.x] + i.samples[p.a.z][p.a.y][p.b.x] * B[p.d.x],
			i.samples[p.a.z][p.b.y][p.a.x] * A[p.d.x] + i.samples[p.a.z][p.b.y][p.b.x] * B[p.d.x],
			i.samples[p.b.z][p.a.y][p.a.x] * A[p.d.x] + i.samples[p.b.z][p.a.y][p.b.x] * B[p.d.x],
			i.samples[p.b.z][p.b.y][p.a.x] * A[p.d.x] + i.samples[p.b.z][p.b.y][p.b.x] * B[p.d.x],
		};
		const float l2[2] = {
			l1[0] * A[p.d.y] + l1[1] * B[p.d.y],
			l1[2] * A[p.d.y] + l1[3] * B[p.d.y],
		};
		return l2[0] * A[p.d.z] + l2[1] * B[p.d.z];
	}

};

void Generator::operator ()(Chunk &chunk) const noexcept {
	ExactLocation::Fine coords(chunk.Position() * ExactLocation::Extent());
	coords += 0.5f;
	ValueField field {
		{ solidity_noise, coords, config.solidity },
		{ humidity_noise, coords, config.humidity },
		{ temperature_noise, coords, config.temperature },
		{ richness_noise, coords, config.richness },
		{ random_noise, coords, config.randomness },
	};
	for (int z = 0; z < Chunk::side; ++z) {
		for (int y = 0; y < Chunk::side; ++y) {
			for (int x = 0; x < Chunk::side; ++x) {
				chunk.SetBlock(RoughLocation::Fine(x, y, z), Generate(field, RoughLocation::Fine(x, y, z)));
			}
		}
	}
	chunk.SetGenerated();
}

Block Generator::Generate(const ValueField &field, const glm::ivec3 &pos) const noexcept {
	Parameters params(ValueField::GetParams(pos));
	float solidity = ValueField::Interpolate(field.solidity, params);
	if (solidity < min_solidity) {
		return Block(0);
	}
	float humidity = ValueField::Interpolate(field.humidity, params);
	float temperature = ValueField::Interpolate(field.temperature, params);
	float richness = ValueField::Interpolate(field.richness, params);

	candidates.clear();
	float total = 0.0f;
	for (const BlockType *type : types) {
		if (!type->solidity.Valid(solidity)) continue;
		if (!type->humidity.Valid(humidity)) continue;
		if (!type->temperature.Valid(temperature)) continue;
		if (!type->richness.Valid(richness)) continue;
		float solidity_match = type->solidity.Map(solidity);
		float humidity_match = type->humidity.Map(humidity);
		float temperature_match = type->temperature.Map(temperature);
		float richness_match = type->richness.Map(richness);
		float chance = (solidity_match + humidity_match + temperature_match + richness_match) * type->commonness;
		total += chance;
		candidates.emplace_back(type, total);
	}
	if (candidates.empty()) {
		return Block(0);
	}
	float random = ValueField::Interpolate(field.randomness, params);
	// as weird as it sounds, but this is faster tham glm::fract and generates a
	// better distribution than (transformed variants of) erf, erfc, atan, and smoothstep
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

}
