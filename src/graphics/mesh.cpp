#include "BlockMesh.hpp"
#include "EntityMesh.hpp"
#include "PrimitiveMesh.hpp"
#include "SkyBoxMesh.hpp"
#include "SpriteMesh.hpp"

#include "../geometry/primitive.hpp"

#include <algorithm>
#include <iostream>


namespace blank {

void EntityMesh::Update(const Buffer &buf) noexcept {
#ifndef NDEBUG
	if (buf.tex_coords.size() < buf.vertices.size()) {
		std::cerr << "EntityMesh: not enough tex coords!" << std::endl;
	}
	if (buf.hsl_mods.size() < buf.vertices.size()) {
		std::cerr << "BlockMesh: not enough HSL modifiers!" << std::endl;
	}
	if (buf.rgb_mods.size() < buf.vertices.size()) {
		std::cerr << "BlockMesh: not enough RGB modifiers!" << std::endl;
	}
	if (buf.normals.size() < buf.vertices.size()) {
		std::cerr << "EntityMesh: not enough normals!" << std::endl;
	}
#endif

	vao.Bind();
	vao.PushAttribute(ATTRIB_VERTEX, buf.vertices);
	vao.PushAttribute(ATTRIB_TEXCOORD, buf.tex_coords);
	vao.PushAttribute(ATTRIB_HSL, buf.hsl_mods);
	vao.PushAttribute(ATTRIB_RGB, buf.rgb_mods);
	vao.PushAttribute(ATTRIB_NORMAL, buf.normals);
	vao.PushIndices(ATTRIB_INDEX, buf.indices);
}


void EntityMesh::Draw() const noexcept {
	vao.DrawTriangleElements();
}


void BlockMesh::Update(const Buffer &buf) noexcept {
#ifndef NDEBUG
	if (buf.tex_coords.size() < buf.vertices.size()) {
		std::cerr << "BlockMesh: not enough tex coords!" << std::endl;
	}
	if (buf.hsl_mods.size() < buf.vertices.size()) {
		std::cerr << "BlockMesh: not enough HSL modifiers!" << std::endl;
	}
	if (buf.rgb_mods.size() < buf.vertices.size()) {
		std::cerr << "BlockMesh: not enough RGB modifiers!" << std::endl;
	}
	if (buf.lights.size() < buf.vertices.size()) {
		std::cerr << "BlockMesh: not enough lights!" << std::endl;
	}
#endif

	vao.Bind();
	vao.PushAttribute(ATTRIB_VERTEX, buf.vertices);
	vao.PushAttribute(ATTRIB_TEXCOORD, buf.tex_coords);
	vao.PushAttribute(ATTRIB_HSL, buf.hsl_mods);
	vao.PushAttribute(ATTRIB_RGB, buf.rgb_mods);
	vao.PushAttribute(ATTRIB_LIGHT, buf.lights);
	vao.PushIndices(ATTRIB_INDEX, buf.indices);
}


void BlockMesh::Draw() const noexcept {
	vao.DrawTriangleElements();
}


void PrimitiveMesh::Buffer::FillRect(
	float w, float h,
	const glm::vec4 &color,
	const glm::vec2 &pivot
) {
	Clear();
	Reserve(4, 6);

	vertices.emplace_back( -pivot.x,  -pivot.y, 0.0f);
	vertices.emplace_back(w-pivot.x,  -pivot.y, 0.0f);
	vertices.emplace_back( -pivot.x, h-pivot.y, 0.0f);
	vertices.emplace_back(w-pivot.x, h-pivot.y, 0.0f);

	colors.resize(4, color);

	indices.assign({ 0, 2, 1, 1, 2, 3 });
}

void PrimitiveMesh::Buffer::OutlineBox(const AABB &box, const glm::vec4 &color) {
	Clear();
	Reserve(8, 24);

	vertices.emplace_back(box.min.x, box.min.y, box.min.z);
	vertices.emplace_back(box.min.x, box.min.y, box.max.z);
	vertices.emplace_back(box.min.x, box.max.y, box.min.z);
	vertices.emplace_back(box.min.x, box.max.y, box.max.z);
	vertices.emplace_back(box.max.x, box.min.y, box.min.z);
	vertices.emplace_back(box.max.x, box.min.y, box.max.z);
	vertices.emplace_back(box.max.x, box.max.y, box.min.z);
	vertices.emplace_back(box.max.x, box.max.y, box.max.z);

	colors.resize(8, color);

	indices.assign({
		0, 1, 1, 3, 3, 2, 2, 0, // left
		4, 5, 5, 7, 7, 6, 6, 4, // right
		0, 4, 1, 5, 3, 7, 2, 6, // others
	});
}


void PrimitiveMesh::Update(const Buffer &buf) noexcept {
#ifndef NDEBUG
	if (buf.colors.size() < buf.vertices.size()) {
		std::cerr << "PrimitiveMesh: not enough colors!" << std::endl;
	}
#endif

	vao.Bind();
	vao.PushAttribute(ATTRIB_VERTEX, buf.vertices);
	vao.PushAttribute(ATTRIB_COLOR, buf.colors);
	vao.PushIndices(ATTRIB_INDEX, buf.indices);
}


void PrimitiveMesh::DrawLines() noexcept {
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);
	vao.DrawLineElements();
}

void PrimitiveMesh::DrawTriangles() noexcept {
	vao.DrawTriangleElements();
}


void SkyBoxMesh::LoadUnitBox() {
	Buffer buffer;
	buffer.vertices = {
		{  1.0f,  1.0f,  1.0f },
		{  1.0f,  1.0f, -1.0f },
		{  1.0f, -1.0f,  1.0f },
		{  1.0f, -1.0f, -1.0f },
		{ -1.0f,  1.0f,  1.0f },
		{ -1.0f,  1.0f, -1.0f },
		{ -1.0f, -1.0f,  1.0f },
		{ -1.0f, -1.0f, -1.0f },
	};
	buffer.indices = {
		5, 7, 3,  3, 1, 5,
		6, 7, 5,  5, 4, 6,
		3, 2, 0,  0, 1, 3,
		6, 4, 0,  0, 2, 6,
		5, 1, 0,  0, 4, 5,
		7, 6, 3,  3, 6, 2,
	};
	Update(buffer);
}

void SkyBoxMesh::Update(const Buffer &buf) noexcept {
	vao.Bind();
	vao.PushAttribute(ATTRIB_VERTEX, buf.vertices);
	vao.PushIndices(ATTRIB_INDEX, buf.indices);
}

void SkyBoxMesh::Draw() const noexcept {
	vao.DrawTriangleElements();
}


void SpriteMesh::Buffer::LoadRect(
	float w, float h,
	const glm::vec2 &pivot,
	const glm::vec2 &tex_begin,
	const glm::vec2 &tex_end
) {
	Clear();
	Reserve(4, 6);

	vertices.emplace_back( -pivot.x,  -pivot.y, 0.0f);
	vertices.emplace_back(w-pivot.x,  -pivot.y, 0.0f);
	vertices.emplace_back( -pivot.x, h-pivot.y, 0.0f);
	vertices.emplace_back(w-pivot.x, h-pivot.y, 0.0f);

	coords.emplace_back(tex_begin.x, tex_begin.y);
	coords.emplace_back(tex_end.x,   tex_begin.y);
	coords.emplace_back(tex_begin.x, tex_end.y);
	coords.emplace_back(tex_end.x,   tex_end.y);

	indices.assign({ 0, 2, 1, 1, 2, 3 });
}


void SpriteMesh::Update(const Buffer &buf) noexcept {
#ifndef NDEBUG
	if (buf.coords.size() < buf.vertices.size()) {
		std::cerr << "SpriteMesh: not enough coords!" << std::endl;
	}
#endif

	vao.Bind();
	vao.PushAttribute(ATTRIB_VERTEX, buf.vertices);
	vao.PushAttribute(ATTRIB_TEXCOORD, buf.coords);
	vao.PushIndices(ATTRIB_INDEX, buf.indices);
}


void SpriteMesh::Draw() noexcept {
	vao.DrawTriangleElements();
}

}
