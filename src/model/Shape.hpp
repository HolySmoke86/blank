#ifndef BLANK_MODEL_SHAPE_HPP_
#define BLANK_MODEL_SHAPE_HPP_

#include "CollisionBounds.hpp"
#include "../geometry/primitive.hpp"
#include "../graphics/BlockMesh.hpp"
#include "../graphics/EntityMesh.hpp"
#include "../world/Block.hpp"

#include <memory>
#include <vector>
#include <glm/glm.hpp>


namespace blank {

class TokenStreamReader;

class Shape {

public:
	struct Faces {
		bool face[Block::FACE_COUNT];
		Faces &operator =(const Faces &other) noexcept {
			for (int i = 0; i < Block::FACE_COUNT; ++i) {
				face[i] = other.face[i];
			}
			return *this;
		}
		bool operator [](Block::Face f) const noexcept {
			return face[f];
		}
	};


public:
	Shape();

	void Read(TokenStreamReader &);

	bool FaceFilled(Block::Face face) const noexcept {
		return fill[face];
	}

	std::size_t VertexCount() const noexcept { return vertices.size(); }
	std::size_t IndexCount() const noexcept { return indices.size(); }

	const glm::vec3 &VertexNormal(size_t idx) const noexcept {
		return vertices[idx].normal;
	}
	glm::vec3 VertexNormal(size_t idx, const glm::mat4 &M) const noexcept {
		return glm::vec3(M * glm::vec4(VertexNormal(idx), 0.0f));
	}

	void Fill(
		EntityMesh::Buffer &,
		const std::vector<float> &tex_map
	) const;
	void Fill(
		EntityMesh::Buffer &,
		const glm::mat4 &transform,
		const std::vector<float> &tex_map
	) const;
	void Fill(
		BlockMesh::Buffer &,
		const glm::mat4 &transform,
		const std::vector<float> &tex_map,
		std::size_t idx_offset = 0
	) const;

	size_t OutlineCount() const noexcept;
	size_t OutlineIndexCount() const noexcept;
	void Outline(PrimitiveMesh::Buffer &out) const;

	bool Intersects(
		const Ray &,
		const glm::mat4 &,
		float &dist,
		glm::vec3 &normal
	) const noexcept;
	bool Intersects(
		const glm::mat4 &M,
		const AABB &box,
		const glm::mat4 &box_M,
		float &depth,
		glm::vec3 &normal
	) const noexcept;

private:
	static float TexR(const std::vector<float> &, std::size_t) noexcept;

private:
	std::unique_ptr<CollisionBounds> bounds;
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 tex_st;
		std::size_t tex_id;
	};
	std::vector<Vertex> vertices;
	std::vector<std::size_t> indices;
	Faces fill;

};

}

#endif
