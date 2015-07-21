#ifndef BLANK_MODEL_SPRITEMODEL_HPP_
#define BLANK_MODEL_SPRITEMODEL_HPP_

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class SpriteModel {

public:
	using Position = glm::vec3;
	using TexCoord = glm::vec2;
	using Index = unsigned short;

	using Positions = std::vector<Position>;
	using TexCoords = std::vector<TexCoord>;
	using Indices = std::vector<Index>;

public:
	Positions vertices;
	TexCoords coords;
	Indices indices;

public:
	SpriteModel() noexcept;
	~SpriteModel() noexcept;

	SpriteModel(const SpriteModel &) = delete;
	SpriteModel &operator =(const SpriteModel &) = delete;

	void Invalidate() noexcept { dirty = true; }

	void Clear() noexcept;
	void Reserve(int vtx_count, int idx_count);

	void LoadRect(
		float w, float h,
		const glm::vec2 &pivot = glm::vec2(0.0f),
		const glm::vec2 &tex_begin = glm::vec2(0.0f),
		const glm::vec2 &tex_end = glm::vec2(1.0f, 1.0f)
	);

	void Draw() noexcept;

private:
	void Update() noexcept;

private:
	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_TEXCOORD,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	GLuint va;
	GLuint handle[ATTRIB_COUNT];
	bool dirty;

};

}

#endif
