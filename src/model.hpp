#ifndef BLANK_MODEL_HPP_
#define BLANK_MODEL_HPP_

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class Model {

public:
	using Index = unsigned int;

public:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> colors;
	std::vector<glm::vec3> normals;
	std::vector<Index> indices;

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

	GLuint handle[ATTRIB_COUNT];
	bool dirty;

};


class OutlineModel {

public:
	using Index = unsigned short;

public:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> colors;
	std::vector<Index> indices;

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

	GLuint handle[ATTRIB_COUNT];
	bool dirty;

};

}

#endif
