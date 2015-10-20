#include "bounds.hpp"
#include "CollisionBounds.hpp"


namespace blank {

void CollisionBounds::Outline(PrimitiveMesh::Buffer &out) const {
	out.vertices.insert(out.vertices.end(), out_pos.begin(), out_pos.end());
	out.indices.insert(out.indices.end(), out_idx.begin(), out_idx.end());
}

void CollisionBounds::SetOutline(
	const PrimitiveMesh::Positions &pos,
	const PrimitiveMesh::Indices &idx
) {
	out_pos = pos;
	out_idx = idx;
}


CuboidBounds::CuboidBounds(const AABB &b)
: CollisionBounds()
, bb(b) {
	bb.Adjust();
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
