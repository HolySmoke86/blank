#ifndef BLANK_MODEL_ENTITYMODEL_HPP_
#define BLANK_MODEL_ENTITYMODEL_HPP_

#include "../graphics/VertexArray.hpp"

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class EntityModel {

public:
	using Position = glm::vec3;
	using Color = glm::vec3;
	using Normal = glm::vec3;
	using Index = unsigned int;

	using Positions = std::vector<Position>;
	using Colors = std::vector<Color>;
	using Normals = std::vector<Normal>;
	using Indices = std::vector<Index>;

	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_COLOR,
		ATTRIB_NORMAL,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	struct Buffer {

		Positions vertices;
		Colors colors;
		Normals normals;
		Indices indices;

		void Clear() noexcept {
			vertices.clear();
			colors.clear();
			normals.clear();
			indices.clear();
		}

		void Reserve(size_t p, size_t i) {
			vertices.reserve(p);
			colors.reserve(p);
			normals.reserve(p);
			indices.reserve(i);
		}

	};

	using VAO = VertexArray<ATTRIB_COUNT>;

public:
	void Update(const Buffer &) noexcept;

	void Draw() const noexcept;

private:
	VAO vao;

};

}

#endif
