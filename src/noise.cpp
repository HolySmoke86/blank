#include "noise.hpp"

#include <cmath>


namespace {

constexpr float one_third = 1.0f/3.0f;
constexpr float one_sixth = 1.0f/6.0f;

}

namespace blank {

GaloisLFSR::GaloisLFSR(std::uint64_t seed) noexcept
: state(seed) {

}

bool GaloisLFSR::operator ()() noexcept {
	bool result = state & 1;
	state >>= 1;
	if (result) {
		state |= 0x8000000000000000;
		state ^= mask;
	} else {
		state &= 0x7FFFFFFFFFFFFFFF;
	}
	return result;
}


SimplexNoise::SimplexNoise(unsigned int seed) noexcept
: grad({
	{  1.0f,  1.0f,  0.0f },
	{ -1.0f,  1.0f,  0.0f },
	{  1.0f, -1.0f,  0.0f },
	{ -1.0f, -1.0f,  0.0f },
	{  1.0f,  0.0f,  1.0f },
	{ -1.0f,  0.0f,  1.0f },
	{  1.0f,  0.0f, -1.0f },
	{ -1.0f,  0.0f, -1.0f },
	{  0.0f,  1.0f,  1.0f },
	{  0.0f, -1.0f,  1.0f },
	{  0.0f,  1.0f, -1.0f },
	{  0.0f, -1.0f, -1.0f },
}) {
	GaloisLFSR random(seed ^ 0x0123456789ACBDEF);
	for (size_t i = 0; i < 256; ++i) {
		random(perm[i]);
		perm[i + 256] = perm[i];
		perm12[i] = perm[i] % 12;
		perm12[i + 256] = perm12[i];
	}
}


float SimplexNoise::operator ()(const glm::vec3 &in) const noexcept {
	float skew = (in.x + in.y + in.z) * one_third;

	glm::vec3 skewed(glm::floor(in + skew));
	float tr = (skewed.x + skewed.y + skewed.z) * one_sixth;

	glm::vec3 unskewed(skewed - tr);
	glm::vec3 offset[4];
	offset[0] = in - unskewed;

	glm::vec3 second, third;

	if (offset[0].x >= offset[0].y) {
		if (offset[0].y >= offset[0].z) {
			second = { 1.0f, 0.0f, 0.0f };
			third = { 1.0f, 1.0f, 0.0f };
		} else if (offset[0].x >= offset[0].z) {
			second = { 1.0f, 0.0f, 0.0f };
			third = { 1.0f, 0.0f, 1.0f };
		} else {
			second = { 0.0f, 0.0f, 1.0f };
			third = { 1.0f, 0.0f, 1.0f };
		}
	} else if (offset[0].y < offset[0].z) {
		second = { 0.0f, 0.0f, 1.0f };
		third = { 0.0f, 1.0f, 1.0f };
	} else if (offset[0].x < offset[0].z) {
		second = { 0.0f, 1.0f, 0.0f };
		third = { 0.0f, 1.0f, 1.0f };
	} else {
		second = { 0.0f, 1.0f, 0.0f };
		third = { 1.0f, 1.0f, 0.0f };
	}

	offset[1] = offset[0] - second + one_sixth;
	offset[2] = offset[0] - third + one_third;
	offset[3] = offset[0] - 0.5f;

	unsigned char index[3] = {
		(unsigned char)(skewed.x),
		(unsigned char)(skewed.y),
		(unsigned char)(skewed.z),
	};
	unsigned char corner[4] = {
		Perm12(index[0] + Perm(index[1] + Perm(index[2]))),
		Perm12(index[0] + int(second.x) + Perm(index[1] + int(second.y) + Perm(index[2] + int(second.z)))),
		Perm12(index[0] + int(third.x) + Perm(index[1] + int(third.y) + Perm(index[2] + int(third.z)))),
		Perm12(index[0] + 1 + Perm(index[1] + 1 + Perm(index[2] + 1))),
	};
	float n[4];
	float t[4];
	for (size_t i = 0; i < 4; ++i) {
		t[i] = 0.6f - dot(offset[i], offset[i]);
		if (t[i] < 0.0f) {
			n[i] = 0.0f;
		} else {
			t[i] *= t[i];
			n[i] = t[i] * t[i] * dot(Grad(corner[i]), offset[i]);
		}
	}

	return 32.0f * (n[0] + n[1] + n[2] + n[3]);
}


unsigned char SimplexNoise::Perm(size_t idx) const noexcept {
	return perm[idx];
}

unsigned char SimplexNoise::Perm12(size_t idx) const noexcept {
	return perm12[idx];
}

const glm::vec3 &SimplexNoise::Grad(unsigned char idx) const noexcept {
	return grad[idx];
}


WorleyNoise::WorleyNoise(unsigned int seed) noexcept
: seed(seed)
, num_points(8) {

}

float WorleyNoise::operator ()(const glm::vec3 &in) const noexcept {
	glm::vec3 center = floor(in);

	float closest = 1.0f;  // cannot be farther away than 1.0

	for (int z = -1; z <= 1; ++z) {
		for (int y = -1; y <= 1; ++y) {
			for (int x = -1; x <= 1; ++x) {
				glm::vec3 cube(center.x + x, center.y + y, center.z + z);
				unsigned int cube_rand =
					(unsigned(cube.x) * 130223) ^
					(unsigned(cube.y) * 159899) ^
					(unsigned(cube.z) * 190717) ^
					seed;

				for (int i = 0; i < num_points; ++i) {
					glm::vec3 point(cube);
					cube_rand = 190667 * cube_rand + 109807;
					point.x += float(cube_rand % 262144) / 262144.0f;
					cube_rand = 135899 * cube_rand + 189169;
					point.y += float(cube_rand % 262144) / 262144.0f;
					cube_rand = 159739 * cube_rand + 112139;
					point.z += float(cube_rand % 262144) / 262144.0f;

					glm::vec3 diff(in - point);
					float distance = sqrt(dot(diff, diff));
					if (distance < closest) {
						closest = distance;
					}
				}
			}
		}
	}

	// closest ranges (0, 1), so normalizing to (-1,1) is trivial
	// though heavily biased towards lower numbers
	return 2.0f * closest - 1.0f;
}

}
