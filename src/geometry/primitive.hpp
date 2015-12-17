#ifndef BLANK_GEOMETRY_PRIMITIVE_HPP_
#define BLANK_GEOMETRY_PRIMITIVE_HPP_

#include <algorithm>
#include <iosfwd>
#include <glm/glm.hpp>


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
		glm::vec3 high(glm::max(abs(min), abs(max)));
		return length(high);
	}
};

std::ostream &operator <<(std::ostream &, const AABB &);

struct Ray {
	glm::vec3 orig;
	glm::vec3 dir;
};

std::ostream &operator <<(std::ostream &, const Ray &);

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
		const float l = length(normal);
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
