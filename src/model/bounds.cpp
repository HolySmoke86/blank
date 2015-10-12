#include "bounds.hpp"
#include "CollisionBounds.hpp"


namespace blank {

void CollisionBounds::Vertices(
	EntityMesh::Buffer &out,
	float tex_offset
) const {
	for (const auto &pos : vtx_pos) {
		out.vertices.emplace_back(pos);
	}
	for (const auto &coord : vtx_tex_coords) {
		out.tex_coords.emplace_back(coord.x, coord.y, coord.z + tex_offset);
	}
	for (const auto &nrm : vtx_nrm) {
		out.normals.emplace_back(nrm);
	}
	for (auto idx : vtx_idx) {
		out.indices.emplace_back(idx);
	}
}

void CollisionBounds::Vertices(
	EntityMesh::Buffer &out,
	const glm::mat4 &transform,
	float tex_offset,
	EntityMesh::Index idx_offset
) const {
	for (const auto &pos : vtx_pos) {
		out.vertices.emplace_back(transform * glm::vec4(pos, 1.0f));
	}
	for (const auto &coord : vtx_tex_coords) {
		out.tex_coords.emplace_back(coord.x, coord.y, coord.z + tex_offset);
	}
	for (const auto &nrm : vtx_nrm) {
		out.normals.emplace_back(transform * glm::vec4(nrm, 0.0f));
	}
	for (auto idx : vtx_idx) {
		out.indices.emplace_back(idx_offset + idx);
	}
}

void CollisionBounds::Vertices(
	BlockMesh::Buffer &out,
	const glm::mat4 &transform,
	float tex_offset,
	BlockMesh::Index idx_offset
) const {
	for (const auto &pos : vtx_pos) {
		out.vertices.emplace_back(transform * glm::vec4(pos, 1.0f));
	}
	for (const auto &coord : vtx_tex_coords) {
		out.tex_coords.emplace_back(coord.x, coord.y, coord.z + tex_offset);
	}
	for (auto idx : vtx_idx) {
		out.indices.emplace_back(idx_offset + idx);
	}
}

void CollisionBounds::Outline(OutlineMesh::Buffer &out) const {
	out.vertices.insert(out.vertices.end(), out_pos.begin(), out_pos.end());
	out.indices.insert(out.indices.end(), out_idx.begin(), out_idx.end());
}

void CollisionBounds::SetShape(
	const EntityMesh::Positions &pos,
	const EntityMesh::Normals &nrm,
	const EntityMesh::Indices &idx
) {
	vtx_pos = pos;
	vtx_nrm = nrm;
	vtx_idx = idx;
}

void CollisionBounds::SetTexture(
	const BlockMesh::TexCoords &tex_coords
) {
	vtx_tex_coords = tex_coords;
}

void CollisionBounds::SetOutline(
	const OutlineMesh::Positions &pos,
	const OutlineMesh::Indices &idx
) {
	out_pos = pos;
	out_idx = idx;
}


NullBounds::NullBounds()
: CollisionBounds() {

}

bool NullBounds::Intersects(
	const Ray &,
	const glm::mat4 &,
	float &, glm::vec3 &
) const noexcept {
	return false;
}

bool NullBounds::Intersects(
	const glm::mat4 &,
	const AABB &,
	const glm::mat4 &,
	float &,
	glm::vec3 &
) const noexcept {
	return false;
}


CuboidBounds::CuboidBounds(const AABB &b)
: CollisionBounds()
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
	SetTexture({
		{ 0.0f, 1.0f, 0.0f }, // front
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f }, // back
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f }, // top
		{ 0.0f, 1.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f }, // bottom
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f }, // left
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f }, // right
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
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

bool CuboidBounds::Intersects(
	const Ray &ray,
	const glm::mat4 &M,
	float &dist, glm::vec3 &normal
) const noexcept {
	return Intersection(ray, bb, M, &dist, &normal);
}

bool CuboidBounds::Intersects(
	const glm::mat4 &M,
	const AABB &box,
	const glm::mat4 &box_M,
	float &depth,
	glm::vec3 &normal
) const noexcept {
	return Intersection(bb, M, box, box_M, depth, normal);
}


StairBounds::StairBounds(const AABB &bb, const glm::vec2 &clip)
: CollisionBounds()
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
	SetTexture({
		{ 0.0f, 0.5f, 0.0f }, // front, upper
		{ 1.0f, 0.5f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f }, // front, lower
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 0.5f, 0.0f },
		{ 1.0f, 0.5f, 0.0f },
		{ 1.0f, 1.0f, 0.0f }, // back
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f }, // top, upper
		{ 0.0f, 0.5f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.5f, 0.0f },
		{ 0.0f, 0.5f, 0.0f }, // top, lower
		{ 0.0f, 1.0f, 0.0f },
		{ 1.0f, 0.5f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f }, // bottom
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.5f, 0.0f }, // left, upper
		{ 0.5f, 0.5f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.5f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f }, // left, lower
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 0.5f, 0.0f },
		{ 1.0f, 0.5f, 0.0f },
		{ 1.0f, 0.5f, 0.0f }, // right, upper
		{ 1.0f, 0.0f, 0.0f },
		{ 0.5f, 0.5f, 0.0f },
		{ 0.5f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f }, // right, lower
		{ 1.0f, 0.5f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.5f, 0.0f },
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
	});
}

bool StairBounds::Intersects(
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

bool StairBounds::Intersects(
	const glm::mat4 &M,
	const AABB &box,
	const glm::mat4 &box_M,
	float &dist,
	glm::vec3 &normal
) const noexcept {
	bool top_hit, bot_hit;
	float top_dist, bot_dist;
	glm::vec3 top_normal, bot_normal;

	top_hit = Intersection(bot, M, box, box_M, top_dist, top_normal);
	bot_hit = Intersection(top, M, box, box_M, bot_dist, bot_normal);

	if (top_hit) {
		if (bot_hit && bot_dist < top_dist) {
			dist = bot_dist;
			normal = bot_normal;
			return true;
		} else {
			dist = top_dist;
			normal = top_normal;
			return true;
		}
		return true;
	} else if (bot_hit) {
		dist = bot_dist;
		normal = bot_normal;
		return true;
	} else {
		return false;
	}
}

}
