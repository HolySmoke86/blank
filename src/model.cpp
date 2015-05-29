#include "model.hpp"

#include <iostream>


namespace blank {

Model::Model() noexcept
: va(0)
, handle{}
, count(0) {
	glGenVertexArrays(1, &va);
	glGenBuffers(ATTRIB_COUNT, handle);
}

Model::~Model() noexcept {
	glDeleteBuffers(ATTRIB_COUNT, handle);
	glDeleteVertexArrays(1, &va);
}

Model::Model(Model &&other) noexcept
: va(other.va)
, count(other.count) {
	other.va = 0;
	for (int i = 0; i < ATTRIB_COUNT; ++i) {
		handle[i] = other.handle[i];
		other.handle[i] = 0;
	}
}

Model &Model::operator =(Model &&other) noexcept {
	std::swap(va, other.va);
	for (int i = 0; i < ATTRIB_COUNT; ++i) {
		std::swap(handle[i], other.handle[i]);
	}
	count = other.count;
	return *this;
}

void Model::Update(const Buffer &buf) noexcept {
	glBindVertexArray(va);
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, buf.vertices.size() * sizeof(glm::vec3), buf.vertices.data(), GL_STATIC_DRAW);
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
	if (buf.colors.size() < buf.vertices.size()) {
		std::cerr << "Model: not enough colors!" << std::endl;
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_COLOR]);
	glBufferData(GL_ARRAY_BUFFER, buf.colors.size() * sizeof(glm::vec3), buf.colors.data(), GL_STATIC_DRAW);
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
	if (buf.normals.size() < buf.vertices.size()) {
		std::cerr << "Model: not enough normals!" << std::endl;
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, buf.normals.size() * sizeof(glm::vec3), buf.normals.data(), GL_STATIC_DRAW);
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf.indices.size() * sizeof(Index), buf.indices.data(), GL_STATIC_DRAW);
	count = buf.indices.size();
}


void Model::Draw() const noexcept {
	glBindVertexArray(va);
	glDrawElements(
		GL_TRIANGLES,    // how
		count,           // count
		GL_UNSIGNED_INT, // type
		nullptr          // offset
	);
}


BlockModel::BlockModel() noexcept
: va(0)
, handle{}
, count(0) {
	glGenVertexArrays(1, &va);
	glGenBuffers(ATTRIB_COUNT, handle);
}

BlockModel::~BlockModel() noexcept {
	glDeleteBuffers(ATTRIB_COUNT, handle);
	glDeleteVertexArrays(1, &va);
}

BlockModel::BlockModel(BlockModel &&other) noexcept
: va(other.va)
, count(other.count) {
	other.va = 0;
	for (int i = 0; i < ATTRIB_COUNT; ++i) {
		handle[i] = other.handle[i];
		other.handle[i] = 0;
	}
}

BlockModel &BlockModel::operator =(BlockModel &&other) noexcept {
	std::swap(va, other.va);
	for (int i = 0; i < ATTRIB_COUNT; ++i) {
		std::swap(handle[i], other.handle[i]);
	}
	count = other.count;
	return *this;
}

void BlockModel::Update(const Buffer &buf) noexcept {
	glBindVertexArray(va);
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, buf.vertices.size() * sizeof(glm::vec3), buf.vertices.data(), GL_STATIC_DRAW);
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
	if (buf.colors.size() < buf.vertices.size()) {
		std::cerr << "BlockModel: not enough colors!" << std::endl;
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_COLOR]);
	glBufferData(GL_ARRAY_BUFFER, buf.colors.size() * sizeof(glm::vec3), buf.colors.data(), GL_STATIC_DRAW);
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
	if (buf.lights.size() < buf.vertices.size()) {
		std::cerr << "BlockModel: not enough lights!" << std::endl;
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, handle[ATTRIB_LIGHT]);
	glBufferData(GL_ARRAY_BUFFER, buf.lights.size() * sizeof(float), buf.lights.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(ATTRIB_LIGHT);
	glVertexAttribPointer(
		ATTRIB_LIGHT, // location (for shader)
		1,            // size
		GL_FLOAT,     // type
		GL_FALSE,     // normalized
		0,            // stride
		nullptr       // offset
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[ATTRIB_INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf.indices.size() * sizeof(Index), buf.indices.data(), GL_STATIC_DRAW);
	count = buf.indices.size();
}


void BlockModel::Draw() const noexcept {
	glBindVertexArray(va);
	glDrawElements(
		GL_TRIANGLES,    // how
		count,           // count
		GL_UNSIGNED_INT, // type
		nullptr          // offset
	);
}

OutlineModel::OutlineModel() noexcept
: vertices()
, colors()
, indices()
, va(0)
, handle{}
, dirty(false) {
	glGenVertexArrays(1, &va);
	glGenBuffers(ATTRIB_COUNT, handle);
}

OutlineModel::~OutlineModel() noexcept {
	glDeleteBuffers(ATTRIB_COUNT, handle);
	glDeleteVertexArrays(1, &va);
}


void OutlineModel::Clear() noexcept {
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


void OutlineModel::Update() noexcept {
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


void OutlineModel::Draw() noexcept {
	glBindVertexArray(va);

	if (dirty) {
		Update();
	}

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);

	glDrawElements(
		GL_LINES,          // how
		indices.size(),    // count
		GL_UNSIGNED_SHORT, // type
		nullptr            // offset
	);
}

}
