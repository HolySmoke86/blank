#include "shape.hpp"


namespace blank {

size_t NullShape::VertexCount() const {
	return 0;
}

void NullShape::Vertices(std::vector<glm::vec3> &out, const glm::vec3 &pos) const {

}

void NullShape::Normals(std::vector<glm::vec3> &out) const {

}

size_t NullShape::OutlineCount() const {
	return 0;
}

void NullShape::Outline(std::vector<glm::vec3> &out, const glm::vec3 &pos) const {

}

bool NullShape::Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const {
	return false;
}


CuboidShape::CuboidShape(const AABB &b)
: Shape()
, bb(b) {
	bb.Adjust();
}


size_t CuboidShape::VertexCount() const {
	return 36;
}

void CuboidShape::Vertices(std::vector<glm::vec3> &out, const glm::vec3 &pos) const {
	out.reserve(36);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z); // front
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z); // back
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z); // top
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z); // bottom
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z); // left
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z); // right
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
}

void CuboidShape::Normals(std::vector<glm::vec3> &out) const {
	out.reserve(36);
	out.insert(out.end(), 6, glm::vec3( 0.0f,  0.0f,  1.0f)); // front
	out.insert(out.end(), 6, glm::vec3( 0.0f,  0.0f, -1.0f)); // back
	out.insert(out.end(), 6, glm::vec3( 0.0f,  1.0f,  0.0f)); // top
	out.insert(out.end(), 6, glm::vec3( 0.0f, -1.0f,  0.0f)); // bottom
	out.insert(out.end(), 6, glm::vec3(-1.0f,  0.0f,  0.0f)); // left
	out.insert(out.end(), 6, glm::vec3( 1.0f,  0.0f,  0.0f)); // right
}


size_t CuboidShape::OutlineCount() const {
	return 24;
}

void CuboidShape::Outline(std::vector<glm::vec3> &out, const glm::vec3 &pos) const {
	out.reserve(24);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
}

bool CuboidShape::Intersects(const Ray &ray, const glm::mat4 &M, float &dist, glm::vec3 &normal) const {
	return Intersection(ray, bb, M, &dist, &normal);
}


StairShape::StairShape(const AABB &bb, const glm::vec2 &clip)
: top({ { clip.x, clip.y, bb.min.z }, bb.max })
, bot({ bb.min, { bb.max.x, clip.y, bb.max.z } }) {

}


size_t StairShape::VertexCount() const {
	return 60;
}

void StairShape::Vertices(std::vector<glm::vec3> &out, const glm::vec3 &pos) const {
	out.reserve(60);
	out.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + top.max.z); // front, upper
	out.emplace_back(pos.x + top.max.x, pos.y + top.min.y, pos.z + top.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.max.z);
	out.emplace_back(pos.x + top.max.x, pos.y + top.min.y, pos.z + top.max.z);
	out.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z); // front, lower
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + top.min.z); // back, upper
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.min.z);
	out.emplace_back(pos.x + top.max.x, pos.y + top.min.y, pos.z + top.min.z);
	out.emplace_back(pos.x + top.max.x, pos.y + top.min.y, pos.z + top.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.min.z);
	out.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.min.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.min.z); // back, lower
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.min.z); // top, upper
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.max.z);
	out.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.min.z);
	out.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.max.z);
	out.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z); // top, lower
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.min.z); // bottom
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + top.min.z); // left, upper
	out.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + top.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + top.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.min.z); // left, lower
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z); // right
	out.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.min.z);
	out.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.max.z);
}

void StairShape::Normals(std::vector<glm::vec3> &out) const {
	out.reserve(60);
	out.insert(out.end(), 12, glm::vec3( 0.0f,  0.0f,  1.0f)); // front, x2
	out.insert(out.end(), 12, glm::vec3( 0.0f,  0.0f, -1.0f)); // back, x2
	out.insert(out.end(), 12, glm::vec3( 0.0f,  1.0f,  0.0f)); // top, x2
	out.insert(out.end(),  6, glm::vec3( 0.0f, -1.0f,  0.0f)); // bottom
	out.insert(out.end(), 12, glm::vec3(-1.0f,  0.0f,  0.0f)); // left, x2
	out.insert(out.end(),  6, glm::vec3( 1.0f,  0.0f,  0.0f)); // right
}


size_t StairShape::OutlineCount() const {
	return 36;
}

void StairShape::Outline(std::vector<glm::vec3> &out, const glm::vec3 &pos) const {
	out.reserve(36);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.min.z); // bottom
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z); // middle
	out.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + bot.min.z); // top
	out.emplace_back(pos.x + bot.max.x, pos.y + top.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + top.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + top.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + top.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.min.z); // verticals, ltr/btf
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + top.max.y, pos.z + bot.min.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	out.emplace_back(pos.x + bot.max.x, pos.y + top.max.y, pos.z + bot.max.z);
}

bool StairShape::Intersects(const Ray &ray, const glm::mat4 &M, float &dist, glm::vec3 &norm) const {
	float top_dist, bot_dist;
	glm::vec3 top_norm, bot_norm;
	bool top_hit = Intersection(ray, top, M, &top_dist, &top_norm);
	bool bot_hit = Intersection(ray, bot, M, &bot_dist, &bot_norm);

	if (top_hit) {
		if (bot_hit) {
			if (top_dist < bot_dist) {
				dist = top_dist;
				norm = top_norm;
				return true;
			} else {
				dist = bot_dist;
				norm = bot_norm;
				return true;
			}
		} else {
			dist = top_dist;
			norm = top_norm;
			return true;
		}
	} else if (bot_hit) {
		dist = bot_dist;
		norm = bot_norm;
		return true;
	} else {
		return false;
	}
}


}
