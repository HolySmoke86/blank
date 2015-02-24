#include "world.hpp"

#include <limits>


namespace blank {

const BlockType BlockType::DEFAULT;

void BlockType::FillVBO(
	const glm::vec3 &pos,
	std::vector<glm::vec3> &vertices,
	std::vector<glm::vec3> &colors,
	std::vector<glm::vec3> &normals
) const {
	vertices.emplace_back(pos.x    , pos.y    , pos.z + 1); // front
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y    , pos.z    ); // back
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z    );
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z    ); // top
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y    , pos.z    ); // bottom
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z    );
	vertices.emplace_back(pos.x    , pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y    , pos.z    ); // left
	vertices.emplace_back(pos.x    , pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x    , pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z    ); // right
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z + 1);

	colors.insert(colors.end(), 6 * 6, color);

	normals.insert(normals.end(), 6, glm::vec3( 0.0f,  0.0f,  1.0f)); // front
	normals.insert(normals.end(), 6, glm::vec3( 0.0f,  0.0f, -1.0f)); // back
	normals.insert(normals.end(), 6, glm::vec3( 0.0f,  1.0f,  0.0f)); // top
	normals.insert(normals.end(), 6, glm::vec3( 0.0f, -1.0f,  0.0f)); // bottom
	normals.insert(normals.end(), 6, glm::vec3(-1.0f,  0.0f,  0.0f)); // left
	normals.insert(normals.end(), 6, glm::vec3( 1.0f,  0.0f,  0.0f)); // right
}


BlockTypeRegistry::BlockTypeRegistry() {
	Add(BlockType::DEFAULT);
}

int BlockTypeRegistry::Add(const BlockType &t) {
	int id = types.size();
	types.push_back(t);
	types.back().id = id;
	return id;
}


Chunk::Chunk()
: blocks(Size())
, model()
, dirty(false) {

}


void Chunk::Draw() {
	if (dirty) {
		Update();
	}
	model.Draw();
}


bool Chunk::Intersection(
	const Ray &ray,
	const glm::mat4 &M,
	int *blkid,
	float *dist,
	glm::vec3 *normal) const {
	{ // rough check
		const AABB bb{{0, 0, 0}, {Width(), Height(), Depth()}};
		if (!blank::Intersection(ray, bb, M)) {
			return false;
		}
	}

	if (!blkid && !dist && !normal) {
		return true;
	}

	// TODO: should be possible to heavily optimize this
	int id = 0;
	int closest_id = -1;
	float closest_dist = std::numeric_limits<float>::infinity();
	glm::vec3 closest_normal(0, 1, 0);
	for (int z = 0; z < Depth(); ++z) {
		for (int y = 0; y < Height(); ++y) {
			for (int x = 0; x < Width(); ++x, ++id) {
				if (!blocks[id].type->visible) {
					continue;
				}
				const AABB bb{{x, y, z}, {x+1, y+1, z+1}};
				float cur_dist;
				glm::vec3 cur_norm;
				if (blank::Intersection(ray, bb, M, &cur_dist, &cur_norm)) {
					if (cur_dist < closest_dist) {
						closest_id = id;
						closest_dist = cur_dist;
						closest_normal = cur_norm;
					}
				}
			}
		}
	}

	if (closest_id < 0) {
		return false;
	}

	if (blkid) {
		*blkid = closest_id;
	}
	if (dist) {
		*dist = closest_dist;
	}
	if (normal) {
		*normal = closest_normal;
	}
	return true;
}


int Chunk::VertexCount() const {
	// TODO: query blocks as soon as type shapes are implemented
	return Size() * 6 * 6;
}

void Chunk::Update() {
	model.Clear();
	model.Reserve(VertexCount());

	for (int i = 0; i < Size(); ++i) {
		if (blocks[i].type->visible) {
			blocks[i].type->FillModel(ToCoords(i), model);
		}
	}

	model.Invalidate();
	dirty = false;
}

}
