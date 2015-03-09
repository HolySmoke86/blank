#include "shape.hpp"


namespace blank {

NullShape::NullShape()
: Shape(0, 0, 0, 0) {

}

void NullShape::Vertices(
	std::vector<glm::vec3> &,
	std::vector<glm::vec3> &,
	std::vector<Model::Index> &,
	const glm::vec3 &,
	Model::Index
) const {

}

void NullShape::Outline(
	std::vector<glm::vec3> &,
	std::vector<OutlineModel::Index> &,
	const glm::vec3 &,
	OutlineModel::Index
) const {

}

bool NullShape::Intersects(
	const Ray &,
	const glm::mat4 &,
	float &, glm::vec3 &
) const {
	return false;
}


CuboidShape::CuboidShape(const AABB &b)
: Shape(24, 36, 8, 24)
, bb(b) {
	bb.Adjust();
}

void CuboidShape::Vertices(
	std::vector<glm::vec3> &vtx,
	std::vector<glm::vec3> &norm,
	std::vector<Model::Index> &index,
	const glm::vec3 &pos,
	Model::Index idx
) const {
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z); // front
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z); // back
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z); // top
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z); // bottom
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z); // left
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z); // right
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);

	norm.insert(norm.end(), 4, glm::vec3( 0.0f,  0.0f,  1.0f)); // front
	norm.insert(norm.end(), 4, glm::vec3( 0.0f,  0.0f, -1.0f)); // back
	norm.insert(norm.end(), 4, glm::vec3( 0.0f,  1.0f,  0.0f)); // top
	norm.insert(norm.end(), 4, glm::vec3( 0.0f, -1.0f,  0.0f)); // bottom
	norm.insert(norm.end(), 4, glm::vec3(-1.0f,  0.0f,  0.0f)); // left
	norm.insert(norm.end(), 4, glm::vec3( 1.0f,  0.0f,  0.0f)); // right

	index.emplace_back(idx +  0); // front
	index.emplace_back(idx +  1);
	index.emplace_back(idx +  2);
	index.emplace_back(idx +  2);
	index.emplace_back(idx +  1);
	index.emplace_back(idx +  3);
	index.emplace_back(idx +  4); // back
	index.emplace_back(idx +  5);
	index.emplace_back(idx +  6);
	index.emplace_back(idx +  6);
	index.emplace_back(idx +  5);
	index.emplace_back(idx +  7);
	index.emplace_back(idx +  8); // top
	index.emplace_back(idx +  9);
	index.emplace_back(idx + 10);
	index.emplace_back(idx + 10);
	index.emplace_back(idx +  9);
	index.emplace_back(idx + 11);
	index.emplace_back(idx + 12); // bottom
	index.emplace_back(idx + 13);
	index.emplace_back(idx + 14);
	index.emplace_back(idx + 14);
	index.emplace_back(idx + 13);
	index.emplace_back(idx + 15);
	index.emplace_back(idx + 16); // left
	index.emplace_back(idx + 17);
	index.emplace_back(idx + 18);
	index.emplace_back(idx + 18);
	index.emplace_back(idx + 17);
	index.emplace_back(idx + 19);
	index.emplace_back(idx + 20); // right
	index.emplace_back(idx + 21);
	index.emplace_back(idx + 22);
	index.emplace_back(idx + 22);
	index.emplace_back(idx + 21);
	index.emplace_back(idx + 23);
}

void CuboidShape::Outline(
	std::vector<glm::vec3> &vtx,
	std::vector<OutlineModel::Index> &index,
	const glm::vec3 &pos,
	OutlineModel::Index idx
) const {
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z); // back
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z); // front
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	vtx.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);

	index.emplace_back(idx +  0); // back
	index.emplace_back(idx +  1);
	index.emplace_back(idx +  1);
	index.emplace_back(idx +  3);
	index.emplace_back(idx +  3);
	index.emplace_back(idx +  2);
	index.emplace_back(idx +  2);
	index.emplace_back(idx +  0);
	index.emplace_back(idx +  4); // front
	index.emplace_back(idx +  5);
	index.emplace_back(idx +  5);
	index.emplace_back(idx +  7);
	index.emplace_back(idx +  7);
	index.emplace_back(idx +  6);
	index.emplace_back(idx +  6);
	index.emplace_back(idx +  4);
	index.emplace_back(idx +  0); // sides
	index.emplace_back(idx +  4);
	index.emplace_back(idx +  1);
	index.emplace_back(idx +  5);
	index.emplace_back(idx +  2);
	index.emplace_back(idx +  6);
	index.emplace_back(idx +  3);
	index.emplace_back(idx +  7);
}

bool CuboidShape::Intersects(
	const Ray &ray,
	const glm::mat4 &M,
	float &dist, glm::vec3 &normal
) const {
	return Intersection(ray, bb, M, &dist, &normal);
}


StairShape::StairShape(const AABB &bb, const glm::vec2 &clip)
: Shape(40, 60, 12, 36)
, top({ { clip.x, clip.y, bb.min.z }, bb.max })
, bot({ bb.min, { bb.max.x, clip.y, bb.max.z } }) {

}


void StairShape::Vertices(
	std::vector<glm::vec3> &vtx,
	std::vector<glm::vec3> &norm,
	std::vector<Model::Index> &index,
	const glm::vec3 &pos,
	Model::Index idx
) const {
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + top.max.z); // front, upper
	vtx.emplace_back(pos.x + top.max.x, pos.y + top.min.y, pos.z + top.max.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.max.z);
	vtx.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.max.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z); // front, lower
	vtx.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + bot.max.x, pos.y + bot.max.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + top.min.z); // back, upper
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.min.z);
	vtx.emplace_back(pos.x + top.max.x, pos.y + top.min.y, pos.z + top.min.z);
	vtx.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.min.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.min.z); // back, lower
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	vtx.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z);
	vtx.emplace_back(pos.x + bot.max.x, pos.y + bot.max.y, pos.z + bot.min.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.min.z); // top, upper
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.max.z);
	vtx.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.min.z);
	vtx.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.max.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z); // top, lower
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.min.z); // bottom
	vtx.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + top.min.z); // left, upper
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.min.y, pos.z + top.max.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.min.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.max.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.min.z); // left, lower
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z); // right
	vtx.emplace_back(pos.x + bot.max.x, pos.y + top.max.y, pos.z + bot.min.z);
	vtx.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + bot.max.x, pos.y + top.max.y, pos.z + bot.max.z);

	norm.insert(norm.end(), 8, glm::vec3( 0.0f,  0.0f,  1.0f)); // front, x2
	norm.insert(norm.end(), 8, glm::vec3( 0.0f,  0.0f, -1.0f)); // back, x2
	norm.insert(norm.end(), 8, glm::vec3( 0.0f,  1.0f,  0.0f)); // top, x2
	norm.insert(norm.end(), 4, glm::vec3( 0.0f, -1.0f,  0.0f)); // bottom
	norm.insert(norm.end(), 8, glm::vec3(-1.0f,  0.0f,  0.0f)); // left, x2
	norm.insert(norm.end(), 4, glm::vec3( 1.0f,  0.0f,  0.0f)); // right

	index.emplace_back(idx +  0); // front, upper
	index.emplace_back(idx +  1);
	index.emplace_back(idx +  2);
	index.emplace_back(idx +  2);
	index.emplace_back(idx +  1);
	index.emplace_back(idx +  3);
	index.emplace_back(idx +  4); // front, lower
	index.emplace_back(idx +  5);
	index.emplace_back(idx +  6);
	index.emplace_back(idx +  6);
	index.emplace_back(idx +  5);
	index.emplace_back(idx +  7);
	index.emplace_back(idx +  8); // back, upper
	index.emplace_back(idx +  9);
	index.emplace_back(idx + 10);
	index.emplace_back(idx + 10);
	index.emplace_back(idx +  9);
	index.emplace_back(idx + 11);
	index.emplace_back(idx + 12); // back, lower
	index.emplace_back(idx + 13);
	index.emplace_back(idx + 14);
	index.emplace_back(idx + 14);
	index.emplace_back(idx + 13);
	index.emplace_back(idx + 15);
	index.emplace_back(idx + 16); // top, upper
	index.emplace_back(idx + 17);
	index.emplace_back(idx + 18);
	index.emplace_back(idx + 18);
	index.emplace_back(idx + 17);
	index.emplace_back(idx + 19);
	index.emplace_back(idx + 20); // top, lower
	index.emplace_back(idx + 21);
	index.emplace_back(idx + 22);
	index.emplace_back(idx + 22);
	index.emplace_back(idx + 21);
	index.emplace_back(idx + 23);
	index.emplace_back(idx + 24); // bottom
	index.emplace_back(idx + 25);
	index.emplace_back(idx + 26);
	index.emplace_back(idx + 26);
	index.emplace_back(idx + 25);
	index.emplace_back(idx + 27);
	index.emplace_back(idx + 28); // left, upper
	index.emplace_back(idx + 29);
	index.emplace_back(idx + 30);
	index.emplace_back(idx + 30);
	index.emplace_back(idx + 29);
	index.emplace_back(idx + 31);
	index.emplace_back(idx + 32); // left, lower
	index.emplace_back(idx + 33);
	index.emplace_back(idx + 34);
	index.emplace_back(idx + 34);
	index.emplace_back(idx + 33);
	index.emplace_back(idx + 35);
	index.emplace_back(idx + 36); // right
	index.emplace_back(idx + 37);
	index.emplace_back(idx + 38);
	index.emplace_back(idx + 38);
	index.emplace_back(idx + 37);
	index.emplace_back(idx + 39);
}

void StairShape::Outline(
	std::vector<glm::vec3> &vtx,
	std::vector<OutlineModel::Index> &index,
	const glm::vec3 &pos,
	OutlineModel::Index idx
) const {
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.min.z); // bottom
	vtx.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.min.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.min.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + bot.max.x, pos.y + bot.min.y, pos.z + bot.max.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + bot.min.z); // middle
	vtx.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + bot.min.z);
	vtx.emplace_back(pos.x + bot.min.x, pos.y + bot.max.y, pos.z + top.max.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + bot.max.y, pos.z + top.max.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.min.z); // top
	vtx.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.min.z);
	vtx.emplace_back(pos.x + top.min.x, pos.y + top.max.y, pos.z + top.max.z);
	vtx.emplace_back(pos.x + top.max.x, pos.y + top.max.y, pos.z + top.max.z);

	index.emplace_back(idx +  0); // bottom
	index.emplace_back(idx +  1);
	index.emplace_back(idx +  1);
	index.emplace_back(idx +  3);
	index.emplace_back(idx +  3);
	index.emplace_back(idx +  2);
	index.emplace_back(idx +  2);
	index.emplace_back(idx +  0);
	index.emplace_back(idx +  4); // middle
	index.emplace_back(idx +  5);
	index.emplace_back(idx +  5);
	index.emplace_back(idx +  7);
	index.emplace_back(idx +  7);
	index.emplace_back(idx +  6);
	index.emplace_back(idx +  6);
	index.emplace_back(idx +  4);
	index.emplace_back(idx +  8); // top
	index.emplace_back(idx +  9);
	index.emplace_back(idx +  9);
	index.emplace_back(idx + 11);
	index.emplace_back(idx + 11);
	index.emplace_back(idx + 10);
	index.emplace_back(idx + 10);
	index.emplace_back(idx +  8);
	index.emplace_back(idx +  0); // verticals
	index.emplace_back(idx +  4);
	index.emplace_back(idx +  2);
	index.emplace_back(idx +  6);
	index.emplace_back(idx +  5);
	index.emplace_back(idx +  8);
	index.emplace_back(idx +  7);
	index.emplace_back(idx + 10);
	index.emplace_back(idx +  1);
	index.emplace_back(idx +  9);
	index.emplace_back(idx +  3);
	index.emplace_back(idx + 11);
}

bool StairShape::Intersects(
	const Ray &ray,
	const glm::mat4 &M,
	float &dist,
	glm::vec3 &norm
) const {
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
