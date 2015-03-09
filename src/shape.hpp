#ifndef BLANK_SHAPE_HPP_
#define BLANK_SHAPE_HPP_

#include "geometry.hpp"
#include "model.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

struct Shape {

	/// the number of vertices (and normals) this shape has
	size_t VertexCount() const { return vtx; }
	/// the number of vertex indices this shape has
	size_t VertexIndexCount() const { return vtx_idx; }

	/// fill given buffers with this shape's elements with an
	/// optional offset
	virtual void Vertices(
		std::vector<glm::vec3> &vertex,
		std::vector<glm::vec3> &normal,
		std::vector<Model::Index> &index,
		const glm::vec3 &elem_offset = { 0.0f, 0.0f, 0.0f },
		Model::Index idx_offset = 0
	) const = 0;

	/// the number of vertices this shape's outline has
	size_t OutlineCount() const { return outl; }
	/// the number of vertex indices this shape's outline has
	size_t OutlineIndexCount() const { return outl_idx; }

	/// fill given buffers with this shape's outline's elements with
	/// an optional offset
	virtual void Outline(
		std::vector<glm::vec3> &vertex,
		std::vector<OutlineModel::Index> &index,
		const glm::vec3 &offset = { 0.0f, 0.0f, 0.0f },
		OutlineModel::Index idx_offset = 0
	) const = 0;

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
	Shape(size_t vtx, size_t vtx_idx, size_t outl, size_t outl_idx)
	: vtx(vtx), vtx_idx(vtx_idx), outl(outl), outl_idx(outl_idx) { }

private:
	size_t vtx;
	size_t vtx_idx;
	size_t outl;
	size_t outl_idx;

};


class NullShape
: public Shape {

public:
	NullShape();

	void Vertices(
		std::vector<glm::vec3> &vertex,
		std::vector<glm::vec3> &normal,
		std::vector<Model::Index> &index,
		const glm::vec3 &elem_offset = { 0.0f, 0.0f, 0.0f },
		Model::Index idx_offset = 0
	) const override;

	void Outline(
		std::vector<glm::vec3> &vertex,
		std::vector<OutlineModel::Index> &index,
		const glm::vec3 &offset = { 0.0f, 0.0f, 0.0f },
		OutlineModel::Index idx_offset = 0
	) const override;

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const override;

};


class CuboidShape
: public Shape {

public:
	CuboidShape(const AABB &bounds);

	void Vertices(
		std::vector<glm::vec3> &vertex,
		std::vector<glm::vec3> &normal,
		std::vector<Model::Index> &index,
		const glm::vec3 &elem_offset = { 0.0f, 0.0f, 0.0f },
		Model::Index idx_offset = 0
	) const override;

	void Outline(
		std::vector<glm::vec3> &vertex,
		std::vector<OutlineModel::Index> &index,
		const glm::vec3 &offset = { 0.0f, 0.0f, 0.0f },
		OutlineModel::Index idx_offset = 0
	) const override;

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const override;

private:
	AABB bb;

};


class StairShape
: public Shape {

public:
	StairShape(const AABB &bounds, const glm::vec2 &clip);

	void Vertices(
		std::vector<glm::vec3> &vertex,
		std::vector<glm::vec3> &normal,
		std::vector<Model::Index> &index,
		const glm::vec3 &elem_offset = { 0.0f, 0.0f, 0.0f },
		Model::Index idx_offset = 0
	) const override;

	void Outline(
		std::vector<glm::vec3> &vertex,
		std::vector<OutlineModel::Index> &index,
		const glm::vec3 &offset = { 0.0f, 0.0f, 0.0f },
		OutlineModel::Index idx_offset = 0
	) const override;

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const override;

private:
	AABB top, bot;

};

}

#endif
