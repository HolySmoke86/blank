#ifndef BLANK_MODEL_HPP_
#define BLANK_MODEL_HPP_

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class Model {

public:
	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_COLOR,
		ATTRIB_NORMAL,
		ATTRIB_COUNT,
	};

public:
	explicit Model(
		std::vector<glm::vec3> &&vertices,
		std::vector<glm::vec3> &&colors,
		std::vector<glm::vec3> &&normals);
	~Model();

	void Draw();

private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> colors;
	std::vector<glm::vec3> normals;
	GLuint handle[ATTRIB_COUNT];

};

}

#endif
