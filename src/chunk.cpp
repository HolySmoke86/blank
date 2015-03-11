#include "chunk.hpp"

#include "generator.hpp"

#include <limits>
#include <glm/gtx/transform.hpp>


namespace {

blank::Model::Buffer buf;

}

namespace blank {

Chunk::Chunk(const BlockTypeRegistry &types)
: types(&types)
, blocks()
, model()
, position(0, 0, 0)
, dirty(false) {

}

Chunk::Chunk(Chunk &&other)
: types(other.types)
, blocks(std::move(other.blocks))
, model(std::move(other.model))
, dirty(other.dirty) {

}

Chunk &Chunk::operator =(Chunk &&other) {
	types = other.types;
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
				if (!Type(blocks[id]).visible) {
					continue;
				}
				float cur_dist;
				glm::vec3 cur_norm;
				Block::Pos pos(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f);
				if (Type(blocks[id]).shape->Intersects(ray, glm::translate(M, pos), cur_dist, cur_norm)) {
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


void Chunk::CheckUpdate() {
	if (dirty) {
		Update();
	}
}

void Chunk::Update() {
	int vtx_count = 0, idx_count = 0;
	for (const auto &block : blocks) {
		const Shape *shape = Type(block).shape;
		vtx_count += shape->VertexCount();
		idx_count += shape->VertexIndexCount();
	}
	buf.Clear();
	buf.Reserve(vtx_count, idx_count);

	Model::Index vtx_counter = 0;
	for (size_t i = 0; i < Size(); ++i) {
		const BlockType &type = Type(blocks[i]);
		type.FillModel(buf, ToCoords(i), vtx_counter);
		vtx_counter += type.shape->VertexCount();
	}

	model.Update(buf);
	dirty = false;
}


ChunkLoader::ChunkLoader(const BlockTypeRegistry &reg, const Generator &gen)
: base(0, 0, 0)
, reg(reg)
, gen(gen)
, loaded()
, to_generate()
, to_free()
, load_dist(4)
, unload_dist(5) {

}

namespace {

struct ChunkLess {

	explicit ChunkLess(const Chunk::Pos &base)
	: base(base) { }

	bool operator ()(const Chunk::Pos &a, const Chunk::Pos &b) const {
		Chunk::Pos da(base - a);
		Chunk::Pos db(base - b);
		return
			da.x * da.x + da.y * da.y + da.z * da.z <
			db.x * db.x + db.y * db.y + db.z * db.z;
	}

	Chunk::Pos base;

};

}

void ChunkLoader::Generate(const Chunk::Pos &from, const Chunk::Pos &to) {
	for (int z = from.z; z < to.z; ++z) {
		for (int y = from.y; y < to.y; ++y) {
			for (int x = from.x; x < to.x; ++x) {
				Chunk::Pos pos(x, y, z);
				if (Known(pos)) {
					continue;
				} else if (x == 0 && y == 0 && z == 0) {
					loaded.emplace_back(reg);
					loaded.back().Position(pos);
					gen(loaded.back());
				} else {
					to_generate.emplace_back(pos);
				}
			}
		}
	}
	to_generate.sort(ChunkLess(base));
}

Chunk *ChunkLoader::Loaded(const Chunk::Pos &pos) {
	for (Chunk &chunk : loaded) {
		if (chunk.Position() == pos) {
			return &chunk;
		}
	}
	return nullptr;
}

bool ChunkLoader::Queued(const Chunk::Pos &pos) {
	for (const Chunk::Pos &chunk : to_generate) {
		if (chunk == pos) {
			return true;
		}
	}
	return nullptr;
}

bool ChunkLoader::Known(const Chunk::Pos &pos) {
	if (Loaded(pos)) return true;
	return Queued(pos);
}

Chunk &ChunkLoader::ForceLoad(const Chunk::Pos &pos) {
	Chunk *chunk = Loaded(pos);
	if (chunk) {
		return *chunk;
	}

	for (auto iter(to_generate.begin()), end(to_generate.end()); iter != end; ++iter) {
		if (*iter == pos) {
			to_generate.erase(iter);
			break;
		}
	}

	loaded.emplace_back(reg);
	loaded.back().Position(pos);
	gen(loaded.back());
	return loaded.back();
}

void ChunkLoader::Rebase(const Chunk::Pos &new_base) {
	if (new_base == base) {
		return;
	}
	base = new_base;

	// unload far away chunks
	for (auto iter(loaded.begin()), end(loaded.end()); iter != end;) {
		if (std::abs(base.x - iter->Position().x) > unload_dist
				|| std::abs(base.y - iter->Position().y) > unload_dist
				|| std::abs(base.z - iter->Position().z) > unload_dist) {
			auto saved = iter;
			++iter;
			to_free.splice(to_free.end(), loaded, saved);
		} else {
			++iter;
		}
	}
	// abort far away queued chunks
	for (auto iter(to_generate.begin()), end(to_generate.end()); iter != end;) {
		if (std::abs(base.x - iter->x) > unload_dist
				|| std::abs(base.y - iter->y) > unload_dist
				|| std::abs(base.z - iter->z) > unload_dist) {
			iter = to_generate.erase(iter);
		} else {
			++iter;
		}
	}
	// add missing new chunks
	const Chunk::Pos offset(load_dist, load_dist, load_dist);
	Generate(base - offset, base + offset);
}

void ChunkLoader::Update() {
	bool reused = false;
	if (!to_generate.empty()) {
		Chunk::Pos pos(to_generate.front());

		for (auto iter(to_free.begin()), end(to_free.end()); iter != end; ++iter) {
			if (iter->Position() == pos) {
				loaded.splice(loaded.end(), to_free, iter);
				reused = true;
				break;
			}
		}

		if (!reused) {
			if (to_free.empty()) {
				loaded.emplace_back(reg);
			} else {
				loaded.splice(loaded.end(), to_free, to_free.begin());
				reused = true;
			}
			loaded.back().Position(pos);
			gen(loaded.back());
		}
		to_generate.pop_front();
	}

	if (!reused && !to_free.empty()) {
		to_free.pop_front();
	}
}

}
