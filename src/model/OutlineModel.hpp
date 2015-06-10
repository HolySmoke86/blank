#ifndef BLANK_MODEL_OUTLINEMODEL_HPP_
#define BLANK_MODEL_OUTLINEMODEL_HPP_

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

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
