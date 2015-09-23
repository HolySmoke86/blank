#ifndef BLANK_MODEL_SKYBOXMODEL_HPP_
#define BLANK_MODEL_SKYBOXMODEL_HPP_

#include "../graphics/VertexArray.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

class SkyBoxModel {

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

	void Draw() const noexcept;

private:
	VAO vao;

};

}

#endif
