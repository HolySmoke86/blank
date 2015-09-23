#ifndef BLANK_MODEL_SHAPE_HPP_
#define BLANK_MODEL_SHAPE_HPP_

#include "BlockModel.hpp"
#include "EntityModel.hpp"
#include "OutlineModel.hpp"
#include "SkyBoxModel.hpp"

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
		EntityModel::Buffer &out,
		float tex_offset = 0.0f
	) const;
	void Vertices(
		EntityModel::Buffer &out,
		const glm::mat4 &transform,
		float tex_offset = 0.0f,
		EntityModel::Index idx_offset = 0
	) const;
	void Vertices(
		BlockModel::Buffer &out,
		const glm::mat4 &transform,
		float tex_offset = 0.0f,
		BlockModel::Index idx_offset = 0
	) const;
	void Vertices(
		SkyBoxModel::Buffer &out
	) const;

	/// the number of vertices this shape's outline has
	size_t OutlineCount() const { return out_pos.size(); }
	/// the number of vertex indices this shape's outline has
	size_t OutlineIndexCount() const { return out_idx.size(); }

	/// fill given buffers with this shape's outline's elements
	void Outline(OutlineModel::Buffer &out) const;

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
	void SetShape(
		const EntityModel::Positions &pos,
		const EntityModel::Normals &nrm,
		const EntityModel::Indices &idx);
	void SetTexture(
		const BlockModel::TexCoords &tex_coords);
	void SetOutline(
		const OutlineModel::Positions &pos,
		const OutlineModel::Indices &idx);

private:
	EntityModel::Positions vtx_pos;
	EntityModel::Normals vtx_nrm;
	EntityModel::Indices vtx_idx;

	BlockModel::TexCoords vtx_tex_coords;

	OutlineModel::Positions out_pos;
	OutlineModel::Indices out_idx;

};

}

#endif
