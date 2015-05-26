#ifndef BLANK_SHAPE_HPP_
#define BLANK_SHAPE_HPP_

#include "geometry.hpp"
#include "model.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

struct Shape {

	/// the number of vertices (and normals) this shape has
	size_t VertexCount() const { return vtx_pos.size(); }
	/// the number of vertex indices this shape has
	size_t VertexIndexCount() const { return vtx_idx.size(); }

	const Model::Normal &VertexNormal(size_t idx) const { return vtx_nrm[idx]; }
	Model::Normal VertexNormal(
		size_t idx, const glm::mat4 &transform
	) const {
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
	) const = 0;

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


class NullShape
: public Shape {

public:
	NullShape();

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const override;

};


class CuboidShape
: public Shape {

public:
	CuboidShape(const AABB &bounds);

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const override;

private:
	AABB bb;

};


class StairShape
: public Shape {

public:
	StairShape(const AABB &bounds, const glm::vec2 &clip);

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const override;

private:
	AABB top, bot;

};

}

#endif
