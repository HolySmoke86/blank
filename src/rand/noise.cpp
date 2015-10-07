#include "GaloisLFSR.hpp"
#include "SimplexNoise.hpp"
#include "WorleyNoise.hpp"

#include <cmath>


namespace {

constexpr float one_third = 1.0f/3.0f;
constexpr float one_sixth = 1.0f/6.0f;

}

namespace blank {

SimplexNoise::SimplexNoise(std::uint64_t seed) noexcept
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
})
, second_ints({
	             // x>y x>z y>z
	{ 0, 0, 1 }, //  0   0   0  ZYX
	{ 0, 1, 0 }, //  0   0   1  YZX
	{ 0, 0, 1 }, //  0   1   0  illogical, but ZYX
	{ 0, 1, 0 }, //  0   1   1  YXZ
	{ 0, 0, 1 }, //  1   0   0  ZXY
	{ 1, 0, 0 }, //  1   0   1  illogical, but XYZ
	{ 1, 0, 0 }, //  1   1   0  XZY
	{ 1, 0, 0 }, //  1   1   1  XYZ
})
, third_ints({
	             // x>y x>z y>z
	{ 0, 1, 1 }, //  0   0   0  ZYX
	{ 0, 1, 1 }, //  0   0   1  YZX
	{ 0, 1, 1 }, //  0   1   0  illogical, but ZYX
	{ 1, 1, 0 }, //  0   1   1  YXZ
	{ 1, 0, 1 }, //  1   0   0  ZXY
	{ 1, 1, 0 }, //  1   0   1  illogical, but XYZ
	{ 1, 0, 1 }, //  1   1   0  XZY
	{ 1, 1, 0 }, //  1   1   1  XYZ
})
, second_floats({
	                      // x>y x>z y>z
	{ 0.0f, 0.0f, 1.0f }, //  0   0   0  ZYX
	{ 0.0f, 1.0f, 0.0f }, //  0   0   1  YZX
	{ 0.0f, 0.0f, 1.0f }, //  0   1   0  illogical, but ZYX
	{ 0.0f, 1.0f, 0.0f }, //  0   1   1  YXZ
	{ 0.0f, 0.0f, 1.0f }, //  1   0   0  ZXY
	{ 1.0f, 0.0f, 0.0f }, //  1   0   1  illogical, but XYZ
	{ 1.0f, 0.0f, 0.0f }, //  1   1   0  XZY
	{ 1.0f, 0.0f, 0.0f }, //  1   1   1  XYZ
})
, third_floats({
	                      // x>y x>z y>z
	{ 0.0f, 1.0f, 1.0f }, //  0   0   0  ZYX
	{ 0.0f, 1.0f, 1.0f }, //  0   0   1  YZX
	{ 0.0f, 1.0f, 1.0f }, //  0   1   0  illogical, but ZYX
	{ 1.0f, 1.0f, 0.0f }, //  0   1   1  YXZ
	{ 1.0f, 0.0f, 1.0f }, //  1   0   0  ZXY
	{ 1.0f, 1.0f, 0.0f }, //  1   0   1  illogical, but XYZ
	{ 1.0f, 0.0f, 1.0f }, //  1   1   0  XZY
	{ 1.0f, 1.0f, 0.0f }, //  1   1   1  XYZ
}) {
	GaloisLFSR random(seed ^ 0x0123456789ACBDEF);
	unsigned char value;
	for (size_t i = 0; i < 256; ++i) {
		perm[i] = random(value);
		perm[i] &= 0xFF;
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
	glm::vec3 relative(in - unskewed);

	bool x_ge_y = relative.x >= relative.y;
	bool x_ge_z = relative.x >= relative.z;
	bool y_ge_z = relative.y >= relative.z;
	unsigned int st = (x_ge_y << 2) | (x_ge_z << 1) | y_ge_z;

	glm::ivec3 second_int(second_ints[st]);
	glm::ivec3 third_int(third_ints[st]);
	glm::vec3 second_float(second_floats[st]);
	glm::vec3 third_float(third_floats[st]);

	glm::vec3 offset[4] = {
		in - unskewed,
		relative - second_float + one_sixth,
		relative - third_float + one_third,
		relative - 0.5f,
	};

	int index[3] = {
		(int)(skewed.x) & 0xFF,
		(int)(skewed.y) & 0xFF,
		(int)(skewed.z) & 0xFF,
	};

	float n = 0.0f;

	// I know 0.6 is wrong, but for some reason it looks better than 0.5

	// 0
	float t = glm::clamp(0.6f - dot(offset[0], offset[0]), 0.0f, 1.0f);
	t *= t;
	int corner = Perm12(index[0] + Perm(index[1] + Perm(index[2])));
	n += t * t * dot(Grad(corner), offset[0]);

	// 1
	t = glm::clamp(0.6f - dot(offset[1], offset[1]), 0.0f, 1.0f);
	t *= t;
	corner = Perm12(index[0] + second_int.x + Perm(index[1] + second_int.y + Perm(index[2] + second_int.z)));
	n += t * t * dot(Grad(corner), offset[1]);

	// 2
	t = glm::clamp(0.6f - dot(offset[2], offset[2]), 0.0f, 1.0f);
	t *= t;
	corner = Perm12(index[0] + third_int.x + Perm(index[1] + third_int.y + Perm(index[2] + third_int.z)));
	n += t * t * dot(Grad(corner), offset[2]);

	// 3
	t = glm::clamp(0.6f - dot(offset[3], offset[3]), 0.0f, 1.0f);
	t *= t;
	corner = Perm12(index[0] + 1 + Perm(index[1] + 1 + Perm(index[2] + 1)));
	n += t * t * dot(Grad(corner), offset[3]);

	return 32.0f * n;
}


int SimplexNoise::Perm(int idx) const noexcept {
	return perm[idx];
}

int SimplexNoise::Perm12(int idx) const noexcept {
	return perm12[idx];
}

const glm::vec3 &SimplexNoise::Grad(int idx) const noexcept {
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
