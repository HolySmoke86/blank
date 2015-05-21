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
	float t_max = std::numeric_limits<float>::infinity();
	const glm::vec3 aabb_pos(M[3].x, M[3].y, M[3].z);
	const glm::vec3 delta = aabb_pos - ray.orig;

	glm::vec3 t1(t_min, t_min, t_min), t2(t_max, t_max, t_max);

	for (int i = 0; i < 3; ++i) {
		const glm::vec3 axis(M[i].x, M[i].y, M[i].z);
		const float e = glm::dot(axis, delta);
		const float f = glm::dot(axis, ray.dir);

		if (std::abs(f) > std::numeric_limits<float>::epsilon()) {
			t1[i] = (e + aabb.min[i]) / f;
			t2[i] = (e + aabb.max[i]) / f;

			t_min = std::max(t_min, std::min(t1[i], t2[i]));
			t_max = std::min(t_max, std::max(t1[i], t2[i]));

			if (t_max < t_min) {
				return false;
			}
		} else {
			if (aabb.min[i] - e < 0.0f || -aabb.max[i] - e > 0.0f) {
				return false;
			}
		}
	}

	glm::vec3 min_all(min(t1, t2));

	if (dist) {
		*dist = t_min;
	}
	if (normal) {
		glm::vec4 norm(0.0f);
		if (min_all.x > min_all.y) {
			if (min_all.x > min_all.z) {
				norm.x = t2.x < t1.x ? 1 : -1;
			} else {
				norm.z = t2.z < t1.z ? 1 : -1;
			}
		} else if (min_all.y > min_all.z) {
			norm.y = t2.y < t1.y ? 1 : -1;
		} else {
			norm.z = t2.z < t1.z ? 1 : -1;
		}
		norm = M * norm;
		*normal = glm::vec3(norm);
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
