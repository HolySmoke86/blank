#ifndef BLANK_MODEL_SHAPE_HPP_
#define BLANK_MODEL_SHAPE_HPP_

#include "BlockModel.hpp"
#include "EntityModel.hpp"
#include "OutlineModel.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

class AABB;
class Ray;

struct Shape {

	/// the number of vertices (and normals) this shape has
	size_t VertexCount() const noexcept { return vtx_pos.size(); }
	/// the number of vertex indices this shape has
	size_t VertexIndexCount() const noexcept { return vtx_idx.size(); }

	const EntityModel::Normal &VertexNormal(size_t idx) const noexcept { return vtx_nrm[idx]; }
	EntityModel::Normal VertexNormal(
		size_t idx, const glm::mat4 &transform
	) const noexcept {
		return EntityModel::Normal(transform * glm::vec4(vtx_nrm[idx], 0.0f));
	}

	/// fill given buffers with this shape's elements with an
	/// optional transform and offset
	void Vertices(
		EntityModel::Positions &vertex,
		EntityModel::Normals &normal,
		EntityModel::Indices &index
	) const;
	void Vertices(
		EntityModel::Positions &vertex,
		EntityModel::Normals &normal,
		EntityModel::Indices &index,
		const glm::mat4 &transform,
		EntityModel::Index idx_offset = 0
	) const;
	void Vertices(
		BlockModel::Positions &vertex,
		BlockModel::Indices &index,
		const glm::mat4 &transform,
		BlockModel::Index idx_offset = 0
	) const;

	/// the number of vertices this shape's outline has
	size_t OutlineCount() const { return out_pos.size(); }
	/// the number of vertex indices this shape's outline has
	size_t OutlineIndexCount() const { return out_idx.size(); }

	/// fill given buffers with this shape's outline's elements with
	/// an optional offset
	void Outline(
		OutlineModel::Positions &vertex,
		OutlineModel::Indices &index,
		const OutlineModel::Position &offset = { 0.0f, 0.0f, 0.0f },
		OutlineModel::Index idx_offset = 0
	) const;

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
	void SetShape(const EntityModel::Positions &pos, const EntityModel::Normals &nrm, const EntityModel::Indices &idx) {
		vtx_pos = pos;
		vtx_nrm = nrm;
		vtx_idx = idx;
	}
	void SetOutline(const OutlineModel::Positions &pos, const OutlineModel::Indices &idx) {
		out_pos = pos;
		out_idx = idx;
	}

private:
	EntityModel::Positions vtx_pos;
	EntityModel::Normals vtx_nrm;
	EntityModel::Indices vtx_idx;

	OutlineModel::Positions out_pos;
	OutlineModel::Indices out_idx;

};

}

#endif
