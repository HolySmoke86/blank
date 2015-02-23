#include "model.hpp"

#include <iostream>


namespace blank {

Model::Model()
: vertices()
, colors()
, normals()
, handle{}
, dirty(false) {
	glGenBuffers(ATTRIB_COUNT, handle);
}

Model::~Model() {
	glDeleteBuffers(ATTRIB_COUNT, handle);
}


void Model::Clear() {
	vertices.clear();
	colors.clear();
	normals.clear();
	Invalidate();
}

void Model::Reserve(int s) {
	vertices.reserve(s);
	colors.reserve(s);
	normals.reserve(s);
}


void Model::Update() {
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

#ifndef NDEBUG
	if (colors.size() < vertices.size()) {
		std::cerr << "Model: not enough colors!" << std::endl;
		colors.resize(vertices.size(), { 1, 0, 1 });
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_COLOR]);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);

#ifndef NDEBUG
	if (normals.size() < vertices.size()) {
		std::cerr << "Model: not enough normals!" << std::endl;
		normals.resize(vertices.size(), { 0, 1, 0 });
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

	dirty = false;
}


void Model::Draw() {
	if (dirty) {
		Update();
	}

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


OutlineModel::OutlineModel()
: vertices()
, colors()
, handle{}
, dirty(false) {
	glGenBuffers(ATTRIB_COUNT, handle);
}

OutlineModel::~OutlineModel() {
	glDeleteBuffers(ATTRIB_COUNT, handle);
}


void OutlineModel::Clear() {
	vertices.clear();
	colors.clear();
	Invalidate();
}

void OutlineModel::Reserve(int s) {
	vertices.reserve(s);
	colors.reserve(s);
}


void OutlineModel::Update() {
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

#ifndef NDEBUG
	if (colors.size() < vertices.size()) {
		std::cerr << "OutlineModel: not enough colors!" << std::endl;
		colors.resize(vertices.size(), { 1, 0, 1 });
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_COLOR]);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);

	dirty = false;
}


void OutlineModel::Draw() {
	if (dirty) {
		Update();
	}

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

	glLineWidth(2.0f);

	glDrawArrays(
		GL_LINES,       // how
		0,              // start
		vertices.size() // len
	);

	glDisableVertexAttribArray(ATTRIB_COLOR);
	glDisableVertexAttribArray(ATTRIB_VERTEX);
}

}
