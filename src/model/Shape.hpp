#ifndef BLANK_MODEL_SHAPE_HPP_
#define BLANK_MODEL_SHAPE_HPP_

#include "../graphics/BlockMesh.hpp"
#include "../graphics/EntityMesh.hpp"

#include <memory>
#include <vector>
#include <glm/glm.hpp>


namespace blank {

struct CollisionBounds;
class TokenStreamReader;

class Shape {

public:
	Shape();

	void Read(TokenStreamReader &);

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

private:
	static float TexR(const std::vector<float> &, std::size_t) noexcept;

private:
	std::unique_ptr<CollisionBounds> bounds;
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tex_st;
		std::size_t tex_id;
	};
	std::vector<Vertex> vertices;
	std::vector<std::size_t> indices;

};

}

#endif
