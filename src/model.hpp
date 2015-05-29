#ifndef BLANK_MODEL_HPP_
#define BLANK_MODEL_HPP_

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class Model {

public:
	using Position = glm::vec3;
	using Color = glm::vec3;
	using Normal = glm::vec3;
	using Index = unsigned int;

	using Positions = std::vector<Position>;
	using Colors = std::vector<Color>;
	using Normals = std::vector<Normal>;
	using Indices = std::vector<Index>;

public:
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

public:
	Model() noexcept;
	~Model() noexcept;

	Model(const Model &) = delete;
	Model &operator =(const Model &) = delete;

	Model(Model &&) noexcept;
	Model &operator =(Model &&) noexcept;

	void Update(const Buffer &) noexcept;

	void Draw() const noexcept;

private:
	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_COLOR,
		ATTRIB_NORMAL,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	GLuint va;
	GLuint handle[ATTRIB_COUNT];
	size_t count;

};


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
	BlockModel &operator =(const Model &) = delete;

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


class OutlineModel {

public:
	using Position = glm::vec3;
	using Color = glm::vec3;
	using Index = unsigned short;

	using Positions = std::vector<Position>;
	using Colors = std::vector<Color>;
	using Indices = std::vector<Index>;

public:
	Positions vertices;
	Colors colors;
	Indices indices;

public:
	OutlineModel() noexcept;
	~OutlineModel() noexcept;

	OutlineModel(const OutlineModel &) = delete;
	OutlineModel &operator =(const OutlineModel &) = delete;

	void Invalidate() noexcept { dirty = true; }

	void Clear() noexcept;
	void Reserve(int vtx_count, int idx_count);

	void Draw() noexcept;

private:
	void Update() noexcept;

private:
	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_COLOR,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	GLuint va;
	GLuint handle[ATTRIB_COUNT];
	bool dirty;

};

}

#endif
