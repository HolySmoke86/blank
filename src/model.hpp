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
	Positions vertices;
	Colors colors;
	Normals normals;
	Indices indices;

public:
	Model();
	~Model();

	Model(const Model &) = delete;
	Model &operator =(const Model &) = delete;

	Model(Model &&);
	Model &operator =(Model &&);

	void Invalidate() { dirty = true; }

	void Clear();
	void Reserve(int vtx_count, int idx_count);

	void CheckUpdate();
	void Draw();

private:
	void Update();

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
	bool dirty;

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
	OutlineModel();
	~OutlineModel();

	OutlineModel(const OutlineModel &) = delete;
	OutlineModel &operator =(const OutlineModel &) = delete;

	void Invalidate() { dirty = true; }

	void Clear();
	void Reserve(int vtx_count, int idx_count);

	void Draw();

private:
	void Update();

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
