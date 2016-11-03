#ifndef BLANK_MODEL_COLLISIONBOUNDS_HPP_
#define BLANK_MODEL_COLLISIONBOUNDS_HPP_

#include "../graphics/PrimitiveMesh.hpp"
#include "../graphics/glm.hpp"


namespace blank {

class AABB;
class Ray;

struct CollisionBounds {

	/// the number of vertices this shape's outline has
	std::size_t OutlineCount() const { return out_pos.size(); }
	/// the number of vertex indices this shape's outline has
	std::size_t OutlineIndexCount() const { return out_idx.size(); }

	/// fill given buffers with these bounds' outline's elements
	void Outline(PrimitiveMesh::Buffer &out) const;

	/// Check if given ray would pass though this shape if it were
	/// transformed with given matrix.
	/// If true, dist and normal hold the intersection distance and
	/// normal, otherwise their content is undefined.
	virtual bool Intersects(
		const Ray &,
		const glm::mat4 &,
		float &dist,
		glm::vec3 &normal
	) const noexcept = 0;

	/// Check for intersection with given OBB.
	/// The OBB is defined by box and box_M, M is applied to the shape.
	virtual bool Intersects(
		const glm::mat4 &M,
		const AABB &box,
		const glm::mat4 &box_M,
		float &depth,
		glm::vec3 &normal
	) const noexcept = 0;

protected:
	void SetOutline(
		const PrimitiveMesh::Positions &pos,
		const PrimitiveMesh::Indices &idx);

private:
	PrimitiveMesh::Positions out_pos;
	PrimitiveMesh::Indices out_idx;

};

}

#endif
