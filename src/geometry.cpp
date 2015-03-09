#include "geometry.hpp"

#include <limits>


namespace blank {

bool Intersection(
	const Ray &ray,
	const AABB &aabb,
	const glm::mat4 &M,
	float *dist,
	glm::vec3 *normal
) {
	float t_min = 0.0f;
	float t_max = 1.0e5f;
	const glm::vec3 aabb_pos(M[3].x, M[3].y, M[3].z);
	const glm::vec3 delta = aabb_pos - ray.orig;

	glm::vec3 t1(t_min, t_min, t_min), t2(t_max, t_max, t_max);
	bool x_swap = false, y_swap = false, z_swap = false;

	{ // X
		const glm::vec3 xaxis(M[0].x, M[0].y, M[0].z);
		const float e = glm::dot(xaxis, delta);
		const float f = glm::dot(ray.dir, xaxis);

		if (std::abs(f) > std::numeric_limits<float>::epsilon()) {
			t1.x = (e + aabb.min.x) / f;
			t2.x = (e + aabb.max.x) / f;

			if (t1.x > t2.x) {
				std::swap(t1.x, t2.x);
				x_swap = true;
			}
			if (t1.x > t_min) {
				t_min = t1.x;
			}
			if (t2.x < t_max) {
				t_max = t2.x;
			}
			if (t_max < t_min) {
				return false;
			}
		} else {
			if (aabb.min.x - e > 0.0f || aabb.max.x < 0.0f) {
				return false;
			}
		}
	}

	{ // Y
		const glm::vec3 yaxis(M[1].x, M[1].y, M[1].z);
		const float e = glm::dot(yaxis, delta);
		const float f = glm::dot(ray.dir, yaxis);

		if (std::abs(f) > std::numeric_limits<float>::epsilon()) {
			t1.y = (e + aabb.min.y) / f;
			t2.y = (e + aabb.max.y) / f;

			if (t1.y > t2.y) {
				std::swap(t1.y, t2.y);
				y_swap = true;
			}
			if (t1.y > t_min) {
				t_min = t1.y;
			}
			if (t2.y < t_max) {
				t_max = t2.y;
			}
			if (t_max < t_min) {
				return false;
			}
		} else {
			if (aabb.min.y - e > 0.0f || aabb.max.y < 0.0f) {
				return false;
			}
		}
	}

	{ // Z
		const glm::vec3 zaxis(M[2].x, M[2].y, M[2].z);
		const float e = glm::dot(zaxis, delta);
		const float f = glm::dot(ray.dir, zaxis);

		if (std::abs(f) > std::numeric_limits<float>::epsilon()) {
			t1.z = (e + aabb.min.z) / f;
			t2.z = (e + aabb.max.z) / f;

			if (t1.z > t2.z) {
				std::swap(t1.z, t2.z);
				z_swap = true;
			}
			if (t1.z > t_min) {
				t_min = t1.z;
			}
			if (t2.z < t_max) {
				t_max = t2.z;
			}
			if (t_max < t_min) {
				return false;
			}
		} else {
			if (aabb.min.z - e > 0.0f || aabb.max.z < 0.0f) {
				return false;
			}
		}
	}

	if (dist) {
		*dist = t_min;
	}
	if (normal) {
		if (t1.x > t1.y) {
			if (t1.x > t1.z) {
				*normal = glm::vec3(x_swap ? 1 : -1, 0, 0);
			} else {
				*normal = glm::vec3(0, 0, z_swap ? 1 : -1);
			}
		} else if (t1.y > t1.z) {
			*normal = glm::vec3(0, y_swap ? 1 : -1, 0);
		} else {
			*normal = glm::vec3(0, 0, z_swap ? 1 : -1);
		}
	}
	return true;
}

bool CullTest(const AABB &box, const glm::mat4 &MVP) {
	// transform corners into clip space
	glm::vec4 corners[8] = {
		{ box.min.x, box.min.y, box.min.z, 1.0f },
		{ box.min.x, box.min.y, box.max.z, 1.0f },
		{ box.min.x, box.max.y, box.min.z, 1.0f },
		{ box.min.x, box.max.y, box.max.z, 1.0f },
		{ box.max.x, box.min.y, box.min.z, 1.0f },
		{ box.max.x, box.min.y, box.max.z, 1.0f },
		{ box.max.x, box.max.y, box.min.z, 1.0f },
		{ box.max.x, box.max.y, box.max.z, 1.0f },
	};
	for (glm::vec4 &corner : corners) {
		corner = MVP * corner;
		corner /= corner.w;
	}

	int hits[6] = { 0, 0, 0, 0, 0, 0 };

	// check how many corners lie outside
	for (const glm::vec4 &corner : corners) {
		if (corner.x >  1.0f) ++hits[0];
		if (corner.x < -1.0f) ++hits[1];
		if (corner.y >  1.0f) ++hits[2];
		if (corner.y < -1.0f) ++hits[3];
		if (corner.z >  1.0f) ++hits[4];
		if (corner.z < -1.0f) ++hits[5];
	}

	// if all corners are outside any given clip plane, the test is true
	for (int hit : hits) {
		if (hit == 8) return true;
	}

	// otherwise the box might still get culled completely, but can't say for sure ;)
	return false;
}

}
