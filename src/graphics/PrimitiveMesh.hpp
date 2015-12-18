#ifndef BLANK_GRAPHICS_PRIMITIVEMESH_HPP_
#define BLANK_GRAPHICS_PRIMITIVEMESH_HPP_

#include "VertexArray.hpp"

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

struct AABB;

class PrimitiveMesh {

public:
	using Position = glm::vec3;
	using Color = glm::tvec4<unsigned char>;
	using Index = unsigned short;

	using Positions = std::vector<Position>;
	using Colors = std::vector<Color>;
	using Indices = std::vector<Index>;

	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_COLOR,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	struct Buffer {

		Positions vertices;
		Colors colors;
		Indices indices;

		void Clear() noexcept {
			vertices.clear();
			colors.clear();
			indices.clear();
		}

		void Reserve(size_t p, size_t i) {
			vertices.reserve(p);
			colors.reserve(p);
			indices.reserve(i);
		}

		void FillRect(
			float w, float h,
			const Color &color = Color(0),
			const glm::vec2 &pivot = glm::vec2(0.0f)
		);

		void OutlineBox(
			const AABB &,
			const Color &color = Color(0)
		);

	};

	using VAO = VertexArray<ATTRIB_COUNT>;

public:
	void Update(const Buffer &) noexcept;

	bool Empty() const noexcept {
		return vao.Empty();
	}

	void DrawLines() const noexcept;
	void DrawTriangles() const noexcept {
		vao.DrawTriangleElements();
	}

private:
	VAO vao;

};

}

#endif
