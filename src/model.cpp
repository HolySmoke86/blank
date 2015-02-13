#include "model.hpp"


namespace blank {

Model::Model(
	std::vector<glm::vec3> &&vtx,
	std::vector<glm::vec3> &&col,
	std::vector<glm::vec3> &&norm
)
: vertices(vtx)
, colors(col)
, normals(norm)
, handle{} {
	glGenBuffers(ATTRIB_COUNT, handle);

	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_COLOR]);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
}

Model::~Model() {
	glDeleteBuffers(ATTRIB_COUNT, handle);
}


void Model::Draw() {
	glEnableVertexAttribArray(ATTRIB_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_VERTEX]);
	glVertexAttribPointer(
		ATTRIB_VERTEX, // location (for shader)
		3,             // size
		GL_FLOAT,      // type
		GL_FALSE,      // normalized
		0,             // stride
		nullptr        // offset
	);

	glEnableVertexAttribArray(ATTRIB_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_COLOR]);
	glVertexAttribPointer(
		ATTRIB_COLOR,  // location (for shader)
		3,             // size
		GL_FLOAT,      // type
		GL_FALSE,      // normalized
		0,             // stride
		nullptr        // offset
	);

	glEnableVertexAttribArray(ATTRIB_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_NORMAL]);
	glVertexAttribPointer(
		ATTRIB_NORMAL, // location (for shader)
		3,             // size
		GL_FLOAT,      // type
		GL_FALSE,      // normalized
		0,             // stride
		nullptr        // offset
	);

	glDrawArrays(
		GL_TRIANGLES,   // how
		0,              // start
		vertices.size() // len
	);

	glDisableVertexAttribArray(ATTRIB_NORMAL);
	glDisableVertexAttribArray(ATTRIB_COLOR);
	glDisableVertexAttribArray(ATTRIB_VERTEX);
}

}
