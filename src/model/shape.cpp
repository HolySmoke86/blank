#include "Shape.hpp"
#include "ShapeRegistry.hpp"

#include "bounds.hpp"
#include "../io/TokenStreamReader.hpp"

#include <string>

using namespace std;


namespace blank {

Shape::Shape()
: bounds()
, vertices()
, indices()
, fill({ false, false, false, false, false, false }) {

}

void Shape::Read(TokenStreamReader &in) {
	bounds.reset();
	vertices.clear();
	indices.clear();
	fill = { false, false, false, false, false, false };

	string name;
	in.Skip(Token::ANGLE_BRACKET_OPEN);
	while (in.HasMore() && in.Peek().type != Token::ANGLE_BRACKET_CLOSE) {
		in.ReadIdentifier(name);
		in.Skip(Token::EQUALS);
		if (name == "bounds") {
			string bounds_class;
			in.ReadIdentifier(bounds_class);
			in.Skip(Token::PARENTHESIS_OPEN);
			if (bounds_class == "Cuboid") {
				glm::vec3 min;
				glm::vec3 max;
				in.ReadVec(min);
				in.Skip(Token::COMMA);
				in.ReadVec(max);
				bounds.reset(new CuboidBounds(AABB{min, max}));
			} else if (bounds_class == "Stair") {
				glm::vec3 min;
				glm::vec3 max;
				glm::vec2 split;
				in.ReadVec(min);
				in.Skip(Token::COMMA);
				in.ReadVec(max);
				in.Skip(Token::COMMA);
				in.ReadVec(split);
				bounds.reset(new StairBounds(AABB{min, max}, split));
			} else {
				while (in.Peek().type != Token::PARENTHESIS_CLOSE) {
					in.Next();
				}
			}
			in.Skip(Token::PARENTHESIS_CLOSE);

		} else if (name == "vertices") {
			in.Skip(Token::ANGLE_BRACKET_OPEN);
			while (in.HasMore() && in.Peek().type != Token::ANGLE_BRACKET_CLOSE) {
				in.Skip(Token::ANGLE_BRACKET_OPEN);
				Vertex vtx;
				in.ReadVec(vtx.position);
				in.Skip(Token::COMMA);
				in.ReadVec(vtx.normal);
				in.Skip(Token::COMMA);
				in.ReadVec(vtx.tex_st);
				in.Skip(Token::COMMA);
				in.ReadNumber(vtx.tex_id);
				if (in.Peek().type == Token::COMMA) {
					in.Skip(Token::COMMA);
				}
				in.Skip(Token::ANGLE_BRACKET_CLOSE);
				if (in.Peek().type == Token::COMMA) {
					in.Skip(Token::COMMA);
				}
				vertices.push_back(vtx);
			}
			in.Skip(Token::ANGLE_BRACKET_CLOSE);

		} else if (name == "indices") {
			in.Skip(Token::ANGLE_BRACKET_OPEN);
			while (in.HasMore() && in.Peek().type != Token::ANGLE_BRACKET_CLOSE) {
				indices.push_back(in.GetULong());
				if (in.Peek().type == Token::COMMA) {
					in.Skip(Token::COMMA);
				}
			}
			in.Skip(Token::ANGLE_BRACKET_CLOSE);

		} else if (name == "fill") {
			in.Skip(Token::BRACKET_OPEN);
			fill.face[Block::FACE_UP] = in.GetBool();
			in.Skip(Token::COMMA);
			fill.face[Block::FACE_DOWN] = in.GetBool();
			in.Skip(Token::COMMA);
			fill.face[Block::FACE_RIGHT] = in.GetBool();
			in.Skip(Token::COMMA);
			fill.face[Block::FACE_LEFT] = in.GetBool();
			in.Skip(Token::COMMA);
			fill.face[Block::FACE_FRONT] = in.GetBool();
			in.Skip(Token::COMMA);
			fill.face[Block::FACE_BACK] = in.GetBool();
			in.Skip(Token::BRACKET_CLOSE);

		} else {
			// try to skip, might fail though
			while (in.HasMore() && in.Peek().type != Token::SEMICOLON) {
				in.Next();
			}
		}
		in.Skip(Token::SEMICOLON);
	}
	in.Skip(Token::ANGLE_BRACKET_CLOSE);
}

float Shape::TexR(const vector<float> &tex_map, size_t off) noexcept {
	if (off < tex_map.size()) {
		return tex_map[off];
	} else if (!tex_map.empty()) {
		return tex_map.back();
	} else {
		return 0.0f;
	}
}

void Shape::Fill(
	EntityMesh::Buffer &buf,
	const vector<float> &tex_map
) const {
	for (const auto &vtx : vertices) {
		buf.vertices.emplace_back(vtx.position);
		buf.normals.emplace_back(vtx.normal);
		buf.tex_coords.emplace_back(vtx.tex_st.s, vtx.tex_st.t, TexR(tex_map, vtx.tex_id));
	}
	for (auto idx : indices) {
		buf.indices.emplace_back(idx);
	}
}

void Shape::Fill(
	EntityMesh::Buffer &buf,
	const glm::mat4 &transform,
	const vector<float> &tex_map
) const {
	for (const auto &vtx : vertices) {
		buf.vertices.emplace_back(transform * glm::vec4(vtx.position, 1.0f));
		buf.normals.emplace_back(transform * glm::vec4(vtx.normal, 0.0f));
		buf.tex_coords.emplace_back(vtx.tex_st.s, vtx.tex_st.t, TexR(tex_map, vtx.tex_id));
	}
	for (auto idx : indices) {
		buf.indices.emplace_back(idx);
	}
}

void Shape::Fill(
	BlockMesh::Buffer &buf,
	const glm::mat4 &transform,
	const vector<float> &tex_map,
	size_t idx_offset
) const {
	for (const auto &vtx : vertices) {
		buf.vertices.emplace_back(transform * glm::vec4(vtx.position, 1.0f));
		buf.tex_coords.emplace_back(vtx.tex_st.s, vtx.tex_st.t, TexR(tex_map, vtx.tex_id));
	}
	for (auto idx : indices) {
		buf.indices.emplace_back(idx_offset + idx);
	}
}

size_t Shape::OutlineCount() const noexcept {
	if (bounds) {
		return bounds->OutlineCount();
	} else {
		return 0;
	}
}

size_t Shape::OutlineIndexCount() const noexcept {
	if (bounds) {
		return bounds->OutlineIndexCount();
	} else {
		return 0;
	}
}

void Shape::Outline(PrimitiveMesh::Buffer &out) const {
	if (bounds) {
		bounds->Outline(out);
	}
}

bool Shape::Intersects(
	const Ray &ray,
	const glm::mat4 &M,
	float &dist,
	glm::vec3 &normal
) const noexcept {
	if (bounds) {
		return bounds->Intersects(ray, M, dist, normal);
	} else {
		return false;
	}
}

bool Shape::Intersects(
	const glm::mat4 &M,
	const AABB &box,
	const glm::mat4 &box_M,
	float &depth,
	glm::vec3 &normal
) const noexcept {
	if (bounds) {
		return bounds->Intersects(M, box, box_M, depth, normal);
	} else {
		return false;
	}
}


ShapeRegistry::ShapeRegistry()
: shapes() {

}

Shape &ShapeRegistry::Add(const string &name) {
	auto result = shapes.emplace(name, Shape());
	if (result.second) {
		return result.first->second;
	} else {
		throw runtime_error("duplicate shape " + name);
	}
}

Shape &ShapeRegistry::Get(const string &name) {
	auto entry = shapes.find(name);
	if (entry != shapes.end()) {
		return entry->second;
	} else {
		throw runtime_error("unknown shape " + name);
	}
}

const Shape &ShapeRegistry::Get(const string &name) const {
	auto entry = shapes.find(name);
	if (entry != shapes.end()) {
		return entry->second;
	} else {
		throw runtime_error("unknown shape " + name);
	}
}

}
