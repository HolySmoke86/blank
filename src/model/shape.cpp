#include "Shape.hpp"
#include "shapes.hpp"


namespace blank {

void Shape::Vertices(
	Model::Positions &vertex,
	Model::Normals &normal,
	Model::Indices &index
) const {
	for (const auto &pos : vtx_pos) {
		vertex.emplace_back(pos);
	}
	for (const auto &nrm : vtx_nrm) {
		normal.emplace_back(nrm);
	}
	for (auto idx : vtx_idx) {
		index.emplace_back(idx);
	}
}

void Shape::Vertices(
	Model::Positions &vertex,
	Model::Normals &normal,
	Model::Indices &index,
	const glm::mat4 &transform,
	Model::Index idx_offset
) const {
	for (const auto &pos : vtx_pos) {
		vertex.emplace_back(transform * glm::vec4(pos, 1.0f));
	}
	for (const auto &nrm : vtx_nrm) {
		normal.emplace_back(transform * glm::vec4(nrm, 0.0f));
	}
	for (auto idx : vtx_idx) {
		index.emplace_back(idx_offset + idx);
	}
}

void Shape::Vertices(
	BlockModel::Positions &vertex,
	BlockModel::Indices &index,
	const glm::mat4 &transform,
	BlockModel::Index idx_offset
) const {
	for (const auto &pos : vtx_pos) {
		vertex.emplace_back(transform * glm::vec4(pos, 1.0f));
	}
	for (auto idx : vtx_idx) {
		index.emplace_back(idx_offset + idx);
	}
}

void Shape::Outline(
	OutlineModel::Positions &vertex,
	OutlineModel::Indices &index,
	const OutlineModel::Position &elem_offset,
	OutlineModel::Index idx_offset
) const {
	for (const auto &pos : out_pos) {
		vertex.emplace_back(elem_offset + pos);
	}
	for (auto idx : out_idx) {
		index.emplace_back(idx_offset + idx);
	}
}


NullShape::NullShape()
: Shape() {

}

bool NullShape::Intersects(
	const Ray &,
	const glm::mat4 &,
	float &, glm::vec3 &
) const noexcept {
	return false;
}

bool NullShape::Intersects(
	const glm::mat4 &,
	const AABB &,
	const glm::mat4 &
) const noexcept {
	return false;
}


CuboidShape::CuboidShape(const AABB &b)
: Shape()
, bb(b) {
	bb.Adjust();
	SetShape({
		{ bb.min.x, bb.min.y, bb.max.z }, // front
		{ bb.max.x, bb.min.y, bb.max.z },
		{ bb.min.x, bb.max.y, bb.max.z },
		{ bb.max.x, bb.max.y, bb.max.z },
		{ bb.min.x, bb.min.y, bb.min.z }, // back
		{ bb.min.x, bb.max.y, bb.min.z },
		{ bb.max.x, bb.min.y, bb.min.z },
		{ bb.max.x, bb.max.y, bb.min.z },
		{ bb.min.x, bb.max.y, bb.min.z }, // top
		{ bb.min.x, bb.max.y, bb.max.z },
		{ bb.max.x, bb.max.y, bb.min.z },
		{ bb.max.x, bb.max.y, bb.max.z },
		{ bb.min.x, bb.min.y, bb.min.z }, // bottom
		{ bb.max.x, bb.min.y, bb.min.z },
		{ bb.min.x, bb.min.y, bb.max.z },
		{ bb.max.x, bb.min.y, bb.max.z },
		{ bb.min.x, bb.min.y, bb.min.z }, // left
		{ bb.min.x, bb.min.y, bb.max.z },
		{ bb.min.x, bb.max.y, bb.min.z },
		{ bb.min.x, bb.max.y, bb.max.z },
		{ bb.max.x, bb.min.y, bb.min.z }, // right
		{ bb.max.x, bb.max.y, bb.min.z },
		{ bb.max.x, bb.min.y, bb.max.z },
		{ bb.max.x, bb.max.y, bb.max.z },
	}, {
		{  0.0f,  0.0f,  1.0f }, // front
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f, -1.0f }, // back
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  1.0f,  0.0f }, // top
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f, -1.0f,  0.0f }, // bottom
		{  0.0f, -1.0f,  0.0f },
		{  0.0f, -1.0f,  0.0f },
		{  0.0f, -1.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f }, // left
		{ -1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f }, // right
		{  1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
	}, {
		  0,  1,  2,  2,  1,  3, // front
		  4,  5,  6,  6,  5,  7, // back
		  8,  9, 10, 10,  9, 11, // top
		 12, 13, 14, 14, 13, 15, // bottom
		 16, 17, 18, 18, 17, 19, // left
		 20, 21, 22, 22, 21, 23, // right
	});
	SetOutline({
		{ bb.min.x, bb.min.y, bb.min.z }, // back
		{ bb.max.x, bb.min.y, bb.min.z },
		{ bb.min.x, bb.max.y, bb.min.z },
		{ bb.max.x, bb.max.y, bb.min.z },
		{ bb.min.x, bb.min.y, bb.max.z }, // front
		{ bb.max.x, bb.min.y, bb.max.z },
		{ bb.min.x, bb.max.y, bb.max.z },
		{ bb.max.x, bb.max.y, bb.max.z },
	}, {
		0, 1, 1, 3, 3, 2, 2, 0, // back
		4, 5, 5, 7, 7, 6, 6, 4, // front
		0, 4, 1, 5, 2, 6, 3, 7, // sides
	});
}

bool CuboidShape::Intersects(
	const Ray &ray,
	const glm::mat4 &M,
	float &dist, glm::vec3 &normal
) const noexcept {
	return Intersection(ray, bb, M, &dist, &normal);
}

bool CuboidShape::Intersects(
	const glm::mat4 &M,
	const AABB &box,
	const glm::mat4 &box_M
) const noexcept {
	return Intersection(bb, M, box, box_M);
}


StairShape::StairShape(const AABB &bb, const glm::vec2 &clip)
: Shape()
, top({ { bb.min.x, clip.y, bb.min.z }, { bb.max.x, bb.max.y, clip.x } })
, bot({ bb.min, { bb.max.x, clip.y, bb.max.z } }) {
	SetShape({
		{ top.min.x, top.min.y, top.max.z }, // front, upper
		{ top.max.x, top.min.y, top.max.z },
		{ top.min.x, top.max.y, top.max.z },
		{ top.max.x, top.max.y, top.max.z },
		{ bot.min.x, bot.min.y, bot.max.z }, // front, lower
		{ bot.max.x, bot.min.y, bot.max.z },
		{ bot.min.x, bot.max.y, bot.max.z },
		{ bot.max.x, bot.max.y, bot.max.z },
		{ bot.min.x, bot.min.y, bot.min.z }, // back
		{ bot.min.x, top.max.y, bot.min.z },
		{ top.max.x, bot.min.y, bot.min.z },
		{ top.max.x, top.max.y, bot.min.z },
		{ top.min.x, top.max.y, top.min.z }, // top, upper
		{ top.min.x, top.max.y, top.max.z },
		{ top.max.x, top.max.y, top.min.z },
		{ top.max.x, top.max.y, top.max.z },
		{ bot.min.x, bot.max.y, top.max.z }, // top, lower
		{ bot.min.x, bot.max.y, bot.max.z },
		{ bot.max.x, bot.max.y, top.max.z },
		{ bot.max.x, bot.max.y, bot.max.z },
		{ bot.min.x, bot.min.y, bot.min.z }, // bottom
		{ bot.max.x, bot.min.y, bot.min.z },
		{ bot.min.x, bot.min.y, bot.max.z },
		{ bot.max.x, bot.min.y, bot.max.z },
		{ top.min.x, top.min.y, top.min.z }, // left, upper
		{ top.min.x, top.min.y, top.max.z },
		{ top.min.x, top.max.y, top.min.z },
		{ top.min.x, top.max.y, top.max.z },
		{ bot.min.x, bot.min.y, bot.min.z }, // left, lower
		{ bot.min.x, bot.min.y, bot.max.z },
		{ bot.min.x, bot.max.y, bot.min.z },
		{ bot.min.x, bot.max.y, bot.max.z },
		{ top.max.x, top.min.y, top.min.z }, // right, upper
		{ top.max.x, top.max.y, top.min.z },
		{ top.max.x, top.min.y, top.max.z },
		{ top.max.x, top.max.y, top.max.z },
		{ bot.max.x, bot.min.y, bot.min.z }, // right, lower
		{ bot.max.x, bot.max.y, bot.min.z },
		{ bot.max.x, bot.min.y, bot.max.z },
		{ bot.max.x, bot.max.y, bot.max.z },
	}, {
		{  0.0f,  0.0f,  1.0f }, // front x2
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f, -1.0f }, // back
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  1.0f,  0.0f }, // top x2
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f, -1.0f,  0.0f }, // bottom
		{  0.0f, -1.0f,  0.0f },
		{  0.0f, -1.0f,  0.0f },
		{  0.0f, -1.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f }, // left x2
		{ -1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f }, // right x2
		{  1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
	}, {
		 0,  1,  2,  2,  1,  3, // front, upper
		 4,  5,  6,  6,  5,  7, // front, lower
		 8,  9, 10, 10,  9, 11, // back
		12, 13, 14, 14, 13, 15, // top, upper
		16, 17, 18, 18, 17, 19, // top, lower
		20, 21, 22, 22, 21, 23, // bottom
		24, 25, 26, 26, 25, 27, // left, upper
		28, 29, 30, 30, 29, 31, // left, lower
		32, 33, 34, 34, 33, 35, // right, upper
		36, 37, 38, 38, 37, 39, // right, lower
	});
	SetOutline({
		{ bot.min.x, bot.min.y, bot.min.z }, // bottom
		{ bot.max.x, bot.min.y, bot.min.z },
		{ bot.min.x, bot.min.y, bot.max.z },
		{ bot.max.x, bot.min.y, bot.max.z },
		{ bot.min.x, bot.max.y, top.max.z }, // middle
		{ bot.max.x, bot.max.y, top.max.z },
		{ bot.min.x, bot.max.y, bot.max.z },
		{ bot.max.x, bot.max.y, bot.max.z },
		{ top.min.x, top.max.y, top.min.z }, // top
		{ top.max.x, top.max.y, top.min.z },
		{ top.min.x, top.max.y, top.max.z },
		{ top.max.x, top.max.y, top.max.z },
	}, {
		 0,  1,  1,  3,  3,  2,  2,  0, // bottom
		 4,  5,  5,  7,  7,  6,  6,  4, // middle
		 8,  9,  9, 11, 11, 10, 10 , 8, // top
		 0,  8,  4, 10,  2,  6, // verticals, btf
		 1,  9,  5, 11,  3,  7,
	//	 5,  8,  7, 10,
	//	 1,  9,  3, 11,
	});
}

bool StairShape::Intersects(
	const Ray &ray,
	const glm::mat4 &M,
	float &dist,
	glm::vec3 &norm
) const noexcept {
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

bool StairShape::Intersects(
	const glm::mat4 &M,
	const AABB &box,
	const glm::mat4 &box_M
) const noexcept {
	return Intersection(bot, M, box, box_M) || Intersection(top, M, box, box_M);
}

}
