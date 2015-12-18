#ifndef BLANK_GRPAHICS_SPRITEMESH_HPP_
#define BLANK_GRPAHICS_SPRITEMESH_HPP_

#include "VertexArray.hpp"

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class SpriteMesh {

public:
	using Position = glm::vec3;
	using TexCoord = glm::vec2;
	using Index = unsigned short;

	using Positions = std::vector<Position>;
	using TexCoords = std::vector<TexCoord>;
	using Indices = std::vector<Index>;

	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_TEXCOORD,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	struct Buffer {

		Positions vertices;
		TexCoords coords;
		Indices indices;

		void Clear() noexcept {
			vertices.clear();
			coords.clear();
			indices.clear();
		}

		void Reserve(size_t p, size_t i) {
			vertices.reserve(p);
			coords.reserve(p);
			indices.reserve(i);
		}

		void LoadRect(
			float w, float h,
			const glm::vec2 &pivot = glm::vec2(0.0f),
			const glm::vec2 &tex_begin = glm::vec2(0.0f),
			const glm::vec2 &tex_end = glm::vec2(1.0f, 1.0f)
		);

	};

	using VAO = VertexArray<ATTRIB_COUNT>;

public:
	void Update(const Buffer &) noexcept;

	bool Empty() const noexcept {
		return vao.Empty();
	}

	void Draw() const noexcept {
		vao.DrawTriangleElements();
	}

private:
	VAO vao;

};

}

#endif
