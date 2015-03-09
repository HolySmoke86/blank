#include "chunk.hpp"

#include <limits>
#include <glm/gtx/transform.hpp>


namespace blank {

Chunk::Chunk()
: blocks()
, model()
, position(0, 0, 0)
, dirty(false) {

}

Chunk::Chunk(Chunk &&other)
: blocks(std::move(other.blocks))
, model(std::move(other.model))
, dirty(other.dirty) {

}

Chunk &Chunk::operator =(Chunk &&other) {
	blocks = std::move(other.blocks);
	model = std::move(other.model);
	dirty = other.dirty;
	return *this;
}


void Chunk::Allocate() {
	blocks.resize(Size());
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
		if (!blank::Intersection(ray, Bounds(), M)) {
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
				float cur_dist;
				glm::vec3 cur_norm;
				Block::Pos pos(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f);
				if (blocks[id].type->shape->Intersects(ray, glm::translate(M, pos), cur_dist, cur_norm)) {
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

void Chunk::Position(const Pos &pos) {
	position = pos;
}

glm::mat4 Chunk::Transform(const Pos &offset) const {
	return glm::translate((position - offset) * Extent());
}


int Chunk::VertexCount() const {
	int count = 0;
	for (const auto &block : blocks) {
		count += block.type->shape->VertexCount();
	}
	return count;
}

void Chunk::Update() {
	model.Clear();
	model.Reserve(VertexCount());

	for (size_t i = 0; i < Size(); ++i) {
		blocks[i].type->FillModel(ToCoords(i), model);
	}

	model.Invalidate();
	dirty = false;
}


}
