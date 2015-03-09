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

Model::Model(Model &&other)
: vertices(std::move(other.vertices))
, colors(std::move(other.colors))
, normals(std::move(other.normals))
, dirty(other.dirty) {
	for (int i = 0; i < ATTRIB_COUNT; ++i) {
		handle[i] = other.handle[i];
		other.handle[i] = 0;
	}
}

Model &Model::operator =(Model &&other) {
	vertices = std::move(other.vertices);
	colors = std::move(other.colors);
	normals = std::move(other.normals);
	indices = std::move(other.indices);
	for (int i = 0; i < ATTRIB_COUNT; ++i) {
		std::swap(handle[i], other.handle[i]);
	}
	dirty = other.dirty;
	return *this;
}


void Model::Clear() {
	vertices.clear();
	colors.clear();
	normals.clear();
	indices.clear();
	Invalidate();
}

void Model::Reserve(int v, int i) {
	vertices.reserve(v);
	colors.reserve(v);
	normals.reserve(v);
	indices.reserve(i);
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[ATTRIB_INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Index), indices.data(), GL_STATIC_DRAW);

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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[ATTRIB_INDEX]);
	glDrawElements(
		GL_TRIANGLES,      // how
		indices.size(),    // count
		GL_UNSIGNED_INT, // type
		nullptr            // offset
	);

	glDisableVertexAttribArray(ATTRIB_NORMAL);
	glDisableVertexAttribArray(ATTRIB_COLOR);
	glDisableVertexAttribArray(ATTRIB_VERTEX);
}


OutlineModel::OutlineModel()
: vertices()
, colors()
, indices()
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
	indices.clear();
	Invalidate();
}

void OutlineModel::Reserve(int v, int i) {
	vertices.reserve(v);
	colors.reserve(v);
	indices.reserve(i);
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[ATTRIB_INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Index), indices.data(), GL_STATIC_DRAW);

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

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[ATTRIB_INDEX]);
	glDrawElements(
		GL_LINES,          // how
		indices.size(),    // count
		GL_UNSIGNED_SHORT, // type
		nullptr            // offset
	);

	glDisableVertexAttribArray(ATTRIB_COLOR);
	glDisableVertexAttribArray(ATTRIB_VERTEX);
}

}
