#include "Shape.hpp"

#include "bounds.hpp"
#include "../io/TokenStreamReader.hpp"

#include <string>

using namespace std;


namespace blank {

Shape::Shape()
: bounds()
, vertices()
, indices() {

}


void Shape::Read(TokenStreamReader &in) {
	bounds.reset();
	vertices.clear();
	indices.clear();

	string name;
	while (in.HasMore()) {
		in.ReadIdentifier(name);
		in.Skip(Token::EQUALS);
		if (name == "bounds") {
			string bounds_class;
			in.ReadIdentifier(bounds_class);
			in.Skip(Token::BRACKET_OPEN);
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
				while (in.Peek().type != Token::BRACKET_CLOSE) {
					in.Next();
				}
			}
			in.Skip(Token::BRACKET_CLOSE);

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
			}

		} else if (name == "indices") {
			in.Skip(Token::ANGLE_BRACKET_OPEN);
			while (in.HasMore() && in.Peek().type != Token::ANGLE_BRACKET_CLOSE) {
				indices.push_back(in.GetULong());
				if (in.Peek().type == Token::COMMA) {
					in.Skip(Token::COMMA);
				}
			}

		} else {
			// try to skip, might fail though
			while (in.Peek().type != Token::SEMICOLON) {
				in.Next();
			}
		}
		in.Skip(Token::SEMICOLON);
	}
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

}
