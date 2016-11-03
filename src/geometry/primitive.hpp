#ifndef BLANK_GEOMETRY_PRIMITIVE_HPP_
#define BLANK_GEOMETRY_PRIMITIVE_HPP_

#include "../graphics/glm.hpp"

#include <algorithm>
#include <iosfwd>
#include <glm/gtx/norm.hpp>


namespace blank {

struct AABB {
	glm::vec3 min;
	glm::vec3 max;

	void Adjust() noexcept {
		if (max.x < min.x) std::swap(max.x, min.x);
		if (max.y < min.y) std::swap(max.y, min.y);
		if (max.z < min.z) std::swap(max.z, min.z);
	}

	glm::vec3 Center() const noexcept {
		return min + (max - min) * 0.5f;
	}

	/// return distance between origin and farthest vertex
	float OriginRadius() const noexcept {
		glm::vec3 high(glm::max(glm::abs(min), glm::abs(max)));
		return glm::length(high);
	}
};

std::ostream &operator <<(std::ostream &, const AABB &);

// TODO: this should really use setters/getters for dir and inv_dir so
//       manipulating code doesn't "forget" to call Update()
struct Ray {
	glm::vec3 orig;
	glm::vec3 dir;

	glm::vec3 inv_dir;

	void Update() noexcept {
		inv_dir = 1.0f / dir;
	}

	/// get shortest distance of this ray's line to given point
	float Distance(const glm::vec3 &point) const noexcept {
		// d = |(x2-x1)×(x1-x0)|/|x2-x1|
		// where x0 is point, and x1 and x2 are points on the line
		// for derivation, see http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
		// x1 = orig
		// x2-x1 = dir, which means |x2-x1| is 1.0
		return glm::length(glm::cross(dir, orig - point));
	}
	float DistanceSquared(const glm::vec3 &point) const noexcept {
		return glm::length2(glm::cross(dir, orig - point));
	}
};

std::ostream &operator <<(std::ostream &, const Ray &);

/// axis aligned boolean ray/box intersection test
/// if true, dist constains distance from ray's origin to intersection point
bool Intersection(
	const Ray &,
	const AABB &,
	float &dist) noexcept;

/// detailed oriented ray/box intersection test
bool Intersection(
	const Ray &,
	const AABB &,
	const glm::mat4 &M,
	float *dist = nullptr,
	glm::vec3 *normal = nullptr) noexcept;

/// matrices may translate and rotate, but must not scale/shear/etc
/// (basically the first three columns must have unit length)
bool Intersection(
	const AABB &a_box,
	const glm::mat4 &a_m,
	const AABB &b_box,
	const glm::mat4 &b_m,
	float &depth,
	glm::vec3 &normal) noexcept;


struct Plane {
	glm::vec3 normal;
	float dist;

	float &A() noexcept { return normal.x; }
	float &B() noexcept { return normal.y; }
	float &C() noexcept { return normal.z; }
	float &D() noexcept { return dist; }
	float A() const noexcept { return normal.x; }
	float B() const noexcept { return normal.y; }
	float C() const noexcept { return normal.z; }
	float D() const noexcept { return dist; }

	Plane(const glm::vec3 &n, float d)
	: normal(n), dist(d) { }
	Plane(const glm::vec4 &abcd)
	: normal(abcd), dist(abcd.w) { }

	void Normalize() noexcept {
		const float l = glm::length(normal);
		normal /= l;
		dist /= l;
	}
};

std::ostream &operator <<(std::ostream &, const Plane &);

struct Frustum {
	Plane plane[6];
	Plane &Left() noexcept { return plane[0]; }
	Plane &Right() noexcept { return plane[1]; }
	Plane &Bottom() noexcept { return plane[2]; }
	Plane &Top() noexcept { return plane[3]; }
	Plane &Near() noexcept { return plane[4]; }
	Plane &Far() noexcept { return plane[5]; }
	const Plane &Left() const noexcept { return plane[0]; }
	const Plane &Right() const noexcept { return plane[1]; }
	const Plane &Bottom() const noexcept { return plane[2]; }
	const Plane &Top() const noexcept { return plane[3]; }
	const Plane &Near() const noexcept { return plane[4]; }
	const Plane &Far() const noexcept { return plane[5]; }

	/// create frustum from transposed MVP
	Frustum(const glm::mat4 &mat)
	: plane{
		{ mat[3] + mat[0] },
		{ mat[3] - mat[0] },
		{ mat[3] + mat[1] },
		{ mat[3] - mat[1] },
		{ mat[3] + mat[2] },
		{ mat[3] - mat[2] },
	} { }

	void Normalize() noexcept {
		for (Plane &p : plane) {
			p.Normalize();
		}
	}
};

std::ostream &operator <<(std::ostream &, const Plane &);
std::ostream &operator <<(std::ostream &, const Frustum &);

bool CullTest(const AABB &box, const glm::mat4 &) noexcept;
bool CullTest(const AABB &box, const Frustum &) noexcept;

}

#endif
