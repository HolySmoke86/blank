#ifndef BLANK_MODEL_SHAPE_HPP_
#define BLANK_MODEL_SHAPE_HPP_

#include "BlockModel.hpp"
#include "Model.hpp"
#include "OutlineModel.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

class Ray;

struct Shape {

	/// the number of vertices (and normals) this shape has
	size_t VertexCount() const noexcept { return vtx_pos.size(); }
	/// the number of vertex indices this shape has
	size_t VertexIndexCount() const noexcept { return vtx_idx.size(); }

	const Model::Normal &VertexNormal(size_t idx) const noexcept { return vtx_nrm[idx]; }
	Model::Normal VertexNormal(
		size_t idx, const glm::mat4 &transform
	) const noexcept {
		return Model::Normal(transform * glm::vec4(vtx_nrm[idx], 0.0f));
	}

	/// fill given buffers with this shape's elements with an
	/// optional transform and offset
	void Vertices(
		Model::Positions &vertex,
		Model::Normals &normal,
		Model::Indices &index
	) const;
	void Vertices(
		Model::Positions &vertex,
		Model::Normals &normal,
		Model::Indices &index,
		const glm::mat4 &transform,
		Model::Index idx_offset = 0
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

protected:
	void SetShape(const Model::Positions &pos, const Model::Normals &nrm, const Model::Indices &idx) {
		vtx_pos = pos;
		vtx_nrm = nrm;
		vtx_idx = idx;
	}
	void SetOutline(const OutlineModel::Positions &pos, const OutlineModel::Indices &idx) {
		out_pos = pos;
		out_idx = idx;
	}

private:
	Model::Positions vtx_pos;
	Model::Normals vtx_nrm;
	Model::Indices vtx_idx;

	OutlineModel::Positions out_pos;
	OutlineModel::Indices out_idx;

};

}

#endif
