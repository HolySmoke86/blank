#ifndef BLANK_MODEL_BLOCKMODEL_HPP_
#define BLANK_MODEL_BLOCKMODEL_HPP_

#include "../graphics/VertexArray.hpp"

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class BlockModel {

public:
	using Position = glm::vec3;
	using TexCoord = glm::vec3;
	using Color = glm::vec3;
	using Light = float;
	using Index = unsigned int;

	using Positions = std::vector<Position>;
	using TexCoords = std::vector<TexCoord>;
	using Colors = std::vector<Color>;
	using Lights = std::vector<Light>;
	using Indices = std::vector<Index>;

	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_TEXCOORD,
		ATTRIB_COLOR,
		ATTRIB_LIGHT,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	struct Buffer {

		Positions vertices;
		TexCoords tex_coords;
		Colors colors;
		Lights lights;
		Indices indices;

		void Clear() noexcept {
			vertices.clear();
			tex_coords.clear();
			colors.clear();
			lights.clear();
			indices.clear();
		}

		void Reserve(size_t p, size_t i) {
			vertices.reserve(p);
			tex_coords.reserve(p);
			colors.reserve(p);
			lights.reserve(p);
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
