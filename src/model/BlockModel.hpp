#ifndef BLANK_MODEL_BLOCKMODEL_HPP_
#define BLANK_MODEL_BLOCKMODEL_HPP_

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class BlockModel {

public:
	using Position = glm::vec3;
	using Color = glm::vec3;
	using Light = float;
	using Index = unsigned int;

	using Positions = std::vector<Position>;
	using Colors = std::vector<Color>;
	using Lights = std::vector<Light>;
	using Indices = std::vector<Index>;

public:
	struct Buffer {

		Positions vertices;
		Colors colors;
		Lights lights;
		Indices indices;

		void Clear() noexcept {
			vertices.clear();
			colors.clear();
			lights.clear();
			indices.clear();
		}

		void Reserve(size_t p, size_t i) {
			vertices.reserve(p);
			colors.reserve(p);
			lights.reserve(p);
			indices.reserve(i);
		}

	};

public:
	BlockModel() noexcept;
	~BlockModel() noexcept;

	BlockModel(const BlockModel &) = delete;
	BlockModel &operator =(const BlockModel &) = delete;

	BlockModel(BlockModel &&) noexcept;
	BlockModel &operator =(BlockModel &&) noexcept;

	void Update(const Buffer &) noexcept;

	void Draw() const noexcept;

private:
	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_COLOR,
		ATTRIB_LIGHT,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	GLuint va;
	GLuint handle[ATTRIB_COUNT];
	size_t count;

};

}

#endif
