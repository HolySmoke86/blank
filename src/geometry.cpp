#include "geometry.hpp"


namespace blank {

bool Intersection(const Ray &ray, const AABB &aabb, const glm::mat4 &M, float *dist) {
	float t_min = 0.0f;
	float t_max = 1.0e5f;
	const glm::vec3 aabb_pos(M[3].x, M[3].y, M[3].z);
	const glm::vec3 delta = aabb_pos - ray.orig;

	{ // X
		const glm::vec3 xaxis(M[0].x, M[0].y, M[0].z);
		const float e = glm::dot(xaxis, delta);
		const float f = glm::dot(ray.dir, xaxis);

		if (std::abs(f) > 0.001f) {
			float t1 = (e + aabb.min.x) / f;
			float t2 = (e + aabb.max.x) / f;

			if (t1 > t2) {
				std::swap(t1, t2);
			}
			if (t1 > t_min) {
				t_min = t1;
			}
			if (t2 < t_max) {
				t_max = t2;
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

		if (std::abs(f) > 0.001f) {
			float t1 = (e + aabb.min.y) / f;
			float t2 = (e + aabb.max.y) / f;

			if (t1 > t2) {
				std::swap(t1, t2);
			}
			if (t1 > t_min) {
				t_min = t1;
			}
			if (t2 < t_max) {
				t_max = t2;
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

		if (std::abs(f) > 0.001f) {
			float t1 = (e + aabb.min.z) / f;
			float t2 = (e + aabb.max.z) / f;

			if (t1 > t2) {
				std::swap(t1, t2);
			}
			if (t1 > t_min) {
				t_min = t1;
			}
			if (t2 < t_max) {
				t_max = t2;
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
	return true;
}

}
