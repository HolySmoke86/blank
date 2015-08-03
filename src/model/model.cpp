#include "BlockModel.hpp"
#include "EntityModel.hpp"
#include "OutlineModel.hpp"
#include "SpriteModel.hpp"

#include <algorithm>
#include <iostream>


namespace blank {

void EntityModel::Update(const Buffer &buf) noexcept {
#ifndef NDEBUG
	if (buf.colors.size() < buf.vertices.size()) {
		std::cerr << "EntityModel: not enough colors!" << std::endl;
	}
	if (buf.normals.size() < buf.vertices.size()) {
		std::cerr << "EntityModel: not enough normals!" << std::endl;
	}
#endif

	vao.Bind();
	vao.PushAttribute(ATTRIB_VERTEX, buf.vertices);
	vao.PushAttribute(ATTRIB_COLOR, buf.colors);
	vao.PushAttribute(ATTRIB_NORMAL, buf.normals);
	vao.PushIndices(ATTRIB_INDEX, buf.indices);
}


void EntityModel::Draw() const noexcept {
	vao.DrawTriangleElements();
}


void BlockModel::Update(const Buffer &buf) noexcept {
#ifndef NDEBUG
	if (buf.colors.size() < buf.vertices.size()) {
		std::cerr << "BlockModel: not enough colors!" << std::endl;
	}
	if (buf.lights.size() < buf.vertices.size()) {
		std::cerr << "BlockModel: not enough lights!" << std::endl;
	}
#endif

	vao.Bind();
	vao.PushAttribute(ATTRIB_VERTEX, buf.vertices);
	vao.PushAttribute(ATTRIB_COLOR, buf.colors);
	vao.PushAttribute(ATTRIB_LIGHT, buf.lights);
	vao.PushIndices(ATTRIB_INDEX, buf.indices);
}


void BlockModel::Draw() const noexcept {
	vao.DrawTriangleElements();
}


void OutlineModel::Update(const Buffer &buf) noexcept {
#ifndef NDEBUG
	if (buf.colors.size() < buf.vertices.size()) {
		std::cerr << "OutlineModel: not enough colors!" << std::endl;
	}
#endif

	vao.Bind();
	vao.PushAttribute(ATTRIB_VERTEX, buf.vertices);
	vao.PushAttribute(ATTRIB_COLOR, buf.colors);
	vao.PushIndices(ATTRIB_INDEX, buf.indices);
}


void OutlineModel::Draw() noexcept {
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);
	vao.DrawLineElements();
}


void SpriteModel::Buffer::LoadRect(
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


void SpriteModel::Update(const Buffer &buf) noexcept {
#ifndef NDEBUG
	if (buf.coords.size() < buf.vertices.size()) {
		std::cerr << "SpriteModel: not enough coords!" << std::endl;
	}
#endif

	vao.Bind();
	vao.PushAttribute(ATTRIB_VERTEX, buf.vertices);
	vao.PushAttribute(ATTRIB_TEXCOORD, buf.coords);
	vao.PushIndices(ATTRIB_INDEX, buf.indices);
}


void SpriteModel::Draw() noexcept {
	vao.DrawTriangleElements();
}

}
