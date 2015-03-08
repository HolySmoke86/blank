#include "noise.hpp"

#include <cmath>


namespace blank {


SimplexNoise::SimplexNoise(unsigned int seed)
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
	unsigned int val = seed;
	for (size_t i = 0; i < 256; ++i) {
		val = 2346765 * val + 6446345;
		perm[i] = val % 256;
	}
}


float SimplexNoise::operator ()(const glm::vec3 &in) const {
	float skew = (in.x + in.y + in.z) / 3.0f;

	glm::vec3 skewed(std::floor(in.x + skew), std::floor(in.y + skew), std::floor(in.z + skew));
	float tr = (skewed.x + skewed.y + skewed.z) / 6.0f;

	glm::vec3 unskewed(skewed.x - tr, skewed.y - tr, skewed.z - tr);
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

	offset[1] = offset[0] - second + glm::vec3(1.0f/6.0f);
	offset[2] = offset[0] - third + glm::vec3(1.0f/3.0f);
	offset[3] = offset[0] - glm::vec3(0.5f);

	size_t index[3] = {
		unsigned(skewed.x) % 256,
		unsigned(skewed.y) % 256,
		unsigned(skewed.z) % 256,
	};
	size_t corner[4] = {
		Perm(index[0] + Perm(index[1] + Perm(index[2]))),
		Perm(index[0] + second.x + Perm(index[1] + second.y + Perm(index[2] + second.z))),
		Perm(index[0] + third.x + Perm(index[1] + third.y + Perm(index[2] + third.z))),
		Perm(index[0] + 1 + Perm(index[1] + 1 + Perm(index[2] + 1))),
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


unsigned char SimplexNoise::Perm(size_t idx) const {
	return perm[idx % 256];
}

const glm::vec3 &SimplexNoise::Grad(size_t idx) const {
	return grad[idx % 12];
}

}
