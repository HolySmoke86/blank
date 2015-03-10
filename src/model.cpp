#include "model.hpp"

#include <iostream>


namespace blank {

Model::Model()
: vertices()
, colors()
, normals()
, indices()
, va(0)
, handle{}
, dirty(false) {
	glGenVertexArrays(1, &va);
	glGenBuffers(ATTRIB_COUNT, handle);
}

Model::~Model() {
	glDeleteBuffers(ATTRIB_COUNT, handle);
	glDeleteVertexArrays(1, &va);
}

Model::Model(Model &&other)
: vertices(std::move(other.vertices))
, colors(std::move(other.colors))
, normals(std::move(other.normals))
, indices(std::move(other.indices))
, va(other.va)
, dirty(other.dirty) {
	other.va = 0;
	for (int i = 0; i < ATTRIB_COUNT; ++i) {
		handle[i] = other.handle[i];
		other.handle[i] = 0;
	}
}

Model &Model::operator =(Model &&other) {
	std::swap(vertices, other.vertices);
	std::swap(colors, other.colors);
	std::swap(normals, other.normals);
	std::swap(indices, other.indices);
	std::swap(va, other.va);
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


void Model::CheckUpdate() {
	if (dirty) {
		glBindVertexArray(va);
		Update();
	}
}

void Model::Update() {
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(ATTRIB_VERTEX);
	glVertexAttribPointer(
		ATTRIB_VERTEX, // location (for shader)
		3,             // size
		GL_FLOAT,      // type
		GL_FALSE,      // normalized
		0,             // stride
		nullptr        // offset
	);

#ifndef NDEBUG
	if (colors.size() < vertices.size()) {
		std::cerr << "Model: not enough colors!" << std::endl;
		colors.resize(vertices.size(), { 1, 0, 1 });
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_COLOR]);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(ATTRIB_COLOR);
	glVertexAttribPointer(
		ATTRIB_COLOR,  // location (for shader)
		3,             // size
		GL_FLOAT,      // type
		GL_FALSE,      // normalized
		0,             // stride
		nullptr        // offset
	);

#ifndef NDEBUG
	if (normals.size() < vertices.size()) {
		std::cerr << "Model: not enough normals!" << std::endl;
		normals.resize(vertices.size(), { 0, 1, 0 });
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(ATTRIB_NORMAL);
	glVertexAttribPointer(
		ATTRIB_NORMAL, // location (for shader)
		3,             // size
		GL_FLOAT,      // type
		GL_FALSE,      // normalized
		0,             // stride
		nullptr        // offset
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[ATTRIB_INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Index), indices.data(), GL_STATIC_DRAW);

	dirty = false;
}


void Model::Draw() {
	glBindVertexArray(va);

	if (dirty) {
		Update();
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[ATTRIB_INDEX]);
	glDrawElements(
		GL_TRIANGLES,      // how
		indices.size(),    // count
		GL_UNSIGNED_INT, // type
		nullptr            // offset
	);
}


OutlineModel::OutlineModel()
: vertices()
, colors()
, indices()
, va(0)
, handle{}
, dirty(false) {
	glGenVertexArrays(1, &va);
	glGenBuffers(ATTRIB_COUNT, handle);
}

OutlineModel::~OutlineModel() {
	glDeleteBuffers(ATTRIB_COUNT, handle);
	glDeleteVertexArrays(1, &va);
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
	glEnableVertexAttribArray(ATTRIB_VERTEX);
	glVertexAttribPointer(
		ATTRIB_VERTEX, // location (for shader)
		3,             // size
		GL_FLOAT,      // type
		GL_FALSE,      // normalized
		0,             // stride
		nullptr        // offset
	);

#ifndef NDEBUG
	if (colors.size() < vertices.size()) {
		std::cerr << "OutlineModel: not enough colors!" << std::endl;
		colors.resize(vertices.size(), { 1, 0, 1 });
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_COLOR]);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(ATTRIB_COLOR);
	glVertexAttribPointer(
		ATTRIB_COLOR,  // location (for shader)
		3,             // size
		GL_FLOAT,      // type
		GL_FALSE,      // normalized
		0,             // stride
		nullptr        // offset
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[ATTRIB_INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Index), indices.data(), GL_STATIC_DRAW);

	dirty = false;
}


void OutlineModel::Draw() {
	glBindVertexArray(va);

	if (dirty) {
		Update();
	}

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[ATTRIB_INDEX]);
	glDrawElements(
		GL_LINES,          // how
		indices.size(),    // count
		GL_UNSIGNED_SHORT, // type
		nullptr            // offset
	);
}

}
