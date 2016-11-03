#ifndef BLANK_GRAPHICS_SKYBOXMESH_HPP_
#define BLANK_GRAPHICS_SKYBOXMESH_HPP_

#include "glm.hpp"
#include "VertexArray.hpp"

#include <vector>


namespace blank {

class SkyBoxMesh {

public:
	using Position = glm::vec3;
	using Index = unsigned int;

	using Positions = std::vector<Position>;
	using Indices = std::vector<Index>;

	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	struct Buffer {

		Positions vertices;
		Indices indices;

		void Clear() noexcept {
			vertices.clear();
			indices.clear();
		}

		void Reserve(size_t p, size_t i) {
			vertices.reserve(p);
			indices.reserve(i);
		}

	};

	using VAO = VertexArray<ATTRIB_COUNT>;

public:
	void LoadUnitBox();
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
