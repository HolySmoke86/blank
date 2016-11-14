#include "const.hpp"
#include "distance.hpp"
#include "primitive.hpp"
#include "rotation.hpp"

#include <limits>
#include <ostream>
#include <glm/gtx/io.hpp>
#include <glm/gtx/matrix_cross_product.hpp>
#include <glm/gtx/optimum_pow.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

glm::mat3 find_rotation(const glm::vec3 &a, const glm::vec3 &b) noexcept {
	glm::vec3 v(glm::cross(a, b));
	if (iszero(v)) {
		// a and b are parallel
		if (iszero(a - b)) {
			// a and b are identical
			return glm::mat3(1.0f);
		} else {
			// a and b are opposite
			// create arbitrary unit vector perpendicular to a and
			// rotate 180° around it
			glm::vec3 arb(a);
			if (std::abs(a.x - 1.0f) > std::numeric_limits<float>::epsilon()) {
				arb.x += 1.0f;
			} else {
				arb.y += 1.0f;
			}
			glm::vec3 axis(glm::normalize(glm::cross(a, arb)));
			return glm::mat3(glm::rotate(PI, axis));
		}
	}
	float mv = glm::length2(v);
	float c = glm::dot(a, b);
	float f = (1 - c) / mv;
	glm::mat3 vx(glm::matrixCross3(v));
	return glm::mat3(1.0f) + vx + (glm::pow2(vx) * f);
}

std::ostream &operator <<(std::ostream &out, const AABB &box) {
	return out << "AABB(" << box.min << ", " << box.max << ')';
}

std::ostream &operator <<(std::ostream &out, const Ray &ray) {
	return out << "Ray(" << ray.orig << ", " << ray.dir << ')';
}

bool Intersection(
	const Ray &ray,
	const AABB &box,
	float &dist
) noexcept {
	float t_min = 0.0f;
	float t_max = std::numeric_limits<float>::infinity();
	for (int i = 0; i < 3; ++i) {
		float t1 = (box.min[i] - ray.orig[i]) * ray.inv_dir[i];
		float t2 = (box.max[i] - ray.orig[i]) * ray.inv_dir[i];
		t_min = std::max(t_min, std::min(t1, t2));
		t_max = std::min(t_max, std::max(t1, t2));
	}
	dist = t_min;
	return t_max >= t_min;
}

bool Intersection(
	const Ray &ray,
	const AABB &aabb,
	const glm::mat4 &M,
	float *dist,
	glm::vec3 *normal
) noexcept {
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
			if (aabb.min[i] - e > 0.0f || aabb.max[i] - e < 0.0f) {
				return false;
			}
		}
	}

	if (dist) {
		*dist = t_min;
	}
	if (normal) {
		glm::vec3 min_all(glm::min(t1, t2));
		if (min_all.x > min_all.y) {
			if (min_all.x > min_all.z) {
				*normal = glm::vec3(t2.x < t1.x ? 1 : -1, 0, 0);
			} else {
				*normal = glm::vec3(0, 0, t2.z < t1.z ? 1 : -1);
			}
		} else if (min_all.y > min_all.z) {
			*normal = glm::vec3(0, t2.y < t1.y ? 1 : -1, 0);
		} else {
			*normal = glm::vec3(0, 0, t2.z < t1.z ? 1 : -1);
		}
	}
	return true;
}

bool Intersection(
	const AABB &a_box,
	const glm::mat4 &a_m,
	const AABB &b_box,
	const glm::mat4 &b_m,
	float &depth,
	glm::vec3 &normal
) noexcept {
	glm::vec3 a_corners[8] = {
		glm::vec3(a_m * glm::vec4(a_box.min.x, a_box.min.y, a_box.min.z, 1)),
		glm::vec3(a_m * glm::vec4(a_box.min.x, a_box.min.y, a_box.max.z, 1)),
		glm::vec3(a_m * glm::vec4(a_box.min.x, a_box.max.y, a_box.min.z, 1)),
		glm::vec3(a_m * glm::vec4(a_box.min.x, a_box.max.y, a_box.max.z, 1)),
		glm::vec3(a_m * glm::vec4(a_box.max.x, a_box.min.y, a_box.min.z, 1)),
		glm::vec3(a_m * glm::vec4(a_box.max.x, a_box.min.y, a_box.max.z, 1)),
		glm::vec3(a_m * glm::vec4(a_box.max.x, a_box.max.y, a_box.min.z, 1)),
		glm::vec3(a_m * glm::vec4(a_box.max.x, a_box.max.y, a_box.max.z, 1)),
	};

	glm::vec3 b_corners[8] = {
		glm::vec3(b_m * glm::vec4(b_box.min.x, b_box.min.y, b_box.min.z, 1)),
		glm::vec3(b_m * glm::vec4(b_box.min.x, b_box.min.y, b_box.max.z, 1)),
		glm::vec3(b_m * glm::vec4(b_box.min.x, b_box.max.y, b_box.min.z, 1)),
		glm::vec3(b_m * glm::vec4(b_box.min.x, b_box.max.y, b_box.max.z, 1)),
		glm::vec3(b_m * glm::vec4(b_box.max.x, b_box.min.y, b_box.min.z, 1)),
		glm::vec3(b_m * glm::vec4(b_box.max.x, b_box.min.y, b_box.max.z, 1)),
		glm::vec3(b_m * glm::vec4(b_box.max.x, b_box.max.y, b_box.min.z, 1)),
		glm::vec3(b_m * glm::vec4(b_box.max.x, b_box.max.y, b_box.max.z, 1)),
	};

	glm::vec3 axes[15] = {
		glm::vec3(a_m[0]),
		glm::vec3(a_m[1]),
		glm::vec3(a_m[2]),
		glm::vec3(b_m[0]),
		glm::vec3(b_m[1]),
		glm::vec3(b_m[2]),
		glm::normalize(glm::cross(glm::vec3(a_m[0]), glm::vec3(b_m[0]))),
		glm::normalize(glm::cross(glm::vec3(a_m[0]), glm::vec3(b_m[1]))),
		glm::normalize(glm::cross(glm::vec3(a_m[0]), glm::vec3(b_m[2]))),
		glm::normalize(glm::cross(glm::vec3(a_m[1]), glm::vec3(b_m[0]))),
		glm::normalize(glm::cross(glm::vec3(a_m[1]), glm::vec3(b_m[1]))),
		glm::normalize(glm::cross(glm::vec3(a_m[1]), glm::vec3(b_m[2]))),
		glm::normalize(glm::cross(glm::vec3(a_m[2]), glm::vec3(b_m[0]))),
		glm::normalize(glm::cross(glm::vec3(a_m[2]), glm::vec3(b_m[1]))),
		glm::normalize(glm::cross(glm::vec3(a_m[2]), glm::vec3(b_m[2]))),
	};

	depth = std::numeric_limits<float>::infinity();
	int min_axis = 0;

	int cur_axis = 0;
	for (const glm::vec3 &axis : axes) {
		if (glm::any(glm::isnan(axis))) {
			// can result from the cross products if A and B have parallel axes
			++cur_axis;
			continue;
		}
		float a_min = std::numeric_limits<float>::infinity();
		float a_max = -std::numeric_limits<float>::infinity();
		for (const glm::vec3 &corner : a_corners) {
			float val = glm::dot(corner, axis);
			a_min = std::min(a_min, val);
			a_max = std::max(a_max, val);
		}

		float b_min = std::numeric_limits<float>::infinity();
		float b_max = -std::numeric_limits<float>::infinity();
		for (const glm::vec3 &corner : b_corners) {
			float val = glm::dot(corner, axis);
			b_min = std::min(b_min, val);
			b_max = std::max(b_max, val);
		}

		if (a_max < b_min || b_max < a_min) return false;

		float overlap = std::min(a_max, b_max) - std::max(a_min, b_min);
		if (overlap < depth) {
			depth = overlap;
			min_axis = cur_axis;
		}

		++cur_axis;
	}

	normal = axes[min_axis];
	return true;
}


std::ostream &operator <<(std::ostream &out, const Plane &plane) {
	return out << "Plane(" << plane.normal << ", " << plane.dist << ')';
}

std::ostream &operator <<(std::ostream &out, const Frustum &frustum) {
	return out << "Frustum(" << std::endl
		<< "\tleft:   " << frustum.plane[0] << std::endl
		<< "\tright:  " << frustum.plane[1] << std::endl
		<< "\tbottom: " << frustum.plane[2] << std::endl
		<< "\ttop:    " << frustum.plane[3] << std::endl
		<< "\tnear:   " << frustum.plane[4] << std::endl
		<< "\tfar:    " << frustum.plane[5] << std::endl
		<< ')';
}

bool CullTest(const AABB &box, const glm::mat4 &MVP) noexcept {
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

	// check how many corners lie outside
	int hits[6] = { 0, 0, 0, 0, 0, 0 };
	for (glm::vec4 &corner : corners) {
		corner = MVP * corner;
		// replacing this with *= 1/w is effectively more expensive
		corner /= corner.w;
		hits[0] += (corner.x >  1.0f);
		hits[1] += (corner.x < -1.0f);
		hits[2] += (corner.y >  1.0f);
		hits[3] += (corner.y < -1.0f);
		hits[4] += (corner.z >  1.0f);
		hits[5] += (corner.z < -1.0f);
	}

	// if all corners are outside any given clip plane, the test is true
	for (int hit : hits) {
		if (hit == 8) return true;
	}

	// otherwise the box might still get culled completely, but can't say for sure ;)
	return false;
}

bool CullTest(const AABB &box, const Frustum &frustum) noexcept {
	for (const Plane &plane : frustum.plane) {
		const glm::vec3 np(
			((plane.normal.x > 0.0f) ? box.max.x : box.min.x),
			((plane.normal.y > 0.0f) ? box.max.y : box.min.y),
			((plane.normal.z > 0.0f) ? box.max.z : box.min.z)
		);
		const float dp = glm::dot(plane.normal, np);
		// cull if nearest point is on the "outside" side of the plane
		if (dp < -plane.dist) return true;
	}
	return false;
}

}
