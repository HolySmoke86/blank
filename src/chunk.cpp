#include "chunk.hpp"

#include "generator.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <glm/gtx/transform.hpp>


namespace blank {

Chunk::Chunk(const BlockTypeRegistry &types)
: types(&types)
, neighbor{0}
, blocks{}
, light{0}
, model()
, position(0, 0, 0)
, dirty(false) {

}

Chunk::Chunk(Chunk &&other)
: types(other.types)
, model(std::move(other.model))
, position(other.position)
, dirty(other.dirty) {
	std::copy(other.neighbor, other.neighbor + sizeof(neighbor), neighbor);
	std::copy(other.blocks, other.blocks + sizeof(blocks), blocks);
	std::copy(other.light, other.light + sizeof(light), light);
}

Chunk &Chunk::operator =(Chunk &&other) {
	types = other.types;
	std::copy(other.neighbor, other.neighbor + sizeof(neighbor), neighbor);
	std::copy(other.blocks, other.blocks + sizeof(blocks), blocks);
	std::copy(other.light, other.light + sizeof(light), light);
	model = std::move(other.model);
	position = other.position;
	dirty = other.dirty;
	return *this;
}


namespace {

struct SetNode {

	Chunk *chunk;
	Chunk::Pos pos;

	SetNode(Chunk *chunk, Chunk::Pos pos)
	: chunk(chunk), pos(pos) { }

	int Get() const { return chunk->GetLight(pos); }
	void Set(int level) { chunk->SetLight(pos, level); }

	bool HasNext(Block::Face face) {
		const BlockLookup next(chunk, pos, face);
		return next && !next.GetType().block_light;
	}
	SetNode GetNext(Block::Face face) {
		const BlockLookup next(chunk, pos, face);
		return SetNode(&next.GetChunk(), next.GetBlockPos());
	}

};

struct UnsetNode
: public SetNode {

	int level;

	UnsetNode(Chunk *chunk, Chunk::Pos pos)
	: SetNode(chunk, pos), level(Get()) { }

	UnsetNode(const SetNode &set)
	: SetNode(set), level(Get()) { }


	bool HasNext(Block::Face face) {
		const BlockLookup next(chunk, pos, face);
		return next;
	}
	UnsetNode GetNext(Block::Face face) { return UnsetNode(SetNode::GetNext(face)); }

};

std::queue<SetNode> light_queue;
std::queue<UnsetNode> dark_queue;

void work_light() {
	while (!light_queue.empty()) {
		SetNode node = light_queue.front();
		light_queue.pop();

		int level = node.Get() - 1;
		for (int face = 0; face < Block::FACE_COUNT; ++face) {
			if (node.HasNext(Block::Face(face))) {
				SetNode other = node.GetNext(Block::Face(face));
				if (other.Get() < level) {
					other.Set(level);
					light_queue.emplace(other);
				}
			}
		}
	}
}

void work_dark() {
	while (!dark_queue.empty()) {
		UnsetNode node = dark_queue.front();
		dark_queue.pop();

		for (int face = 0; face < Block::FACE_COUNT; ++face) {
			if (node.HasNext(Block::Face(face))) {
				UnsetNode other = node.GetNext(Block::Face(face));
				// TODO: if there a light source here with the same level this will err
				if (other.Get() != 0 && other.Get() < node.level) {
					other.Set(0);
					dark_queue.emplace(other);
				} else {
					light_queue.emplace(other);
				}
			}
		}
	}
}

}

void Chunk::SetBlock(int index, const Block &block) {
	const BlockType &old_type = Type(blocks[index]);
	const BlockType &new_type = Type(block);

	blocks[index] = block;

	if (&old_type == &new_type) return;

	if (new_type.luminosity > old_type.luminosity) {
		// light added
		SetLight(index, new_type.luminosity);
		light_queue.emplace(this, ToPos(index));
		work_light();
	} else if (new_type.luminosity < old_type.luminosity) {
		// light removed
		dark_queue.emplace(this, ToPos(index));
		SetLight(index, 0);
		work_dark();
		SetLight(index, new_type.luminosity);
		light_queue.emplace(this, ToPos(index));
		work_light();
	} else if (new_type.block_light && !old_type.block_light) {
		// obstacle added
		if (GetLight(index) > 0) {
			dark_queue.emplace(this, ToPos(index));
			SetLight(index, 0);
			work_dark();
			work_light();
		}
	} else if (!new_type.block_light && old_type.block_light) {
		// obstacle removed
		int level = 0;
		for (int face = 0; face < Block::FACE_COUNT; ++face) {
			BlockLookup next_block(this, ToPos(index), Block::Face(face));
			if (next_block) {
				level = std::min(level, next_block.GetLight());
			}
		}
		if (level > 1) {
			SetLight(index, level - 1);
			light_queue.emplace(this, ToPos(index));
			work_light();
		}
	}
}

void Chunk::SetNeighbor(Chunk &other) {
	if (other.position == position + Pos(-1, 0, 0)) {
		if (neighbor[Block::FACE_LEFT] != &other) {
			neighbor[Block::FACE_LEFT] = &other;
			other.neighbor[Block::FACE_RIGHT] = this;
			for (int z = 0; z < Depth(); ++z) {
				for (int y = 0; y < Height(); ++y) {
					Pos my_pos(0, y, z);
					Pos other_pos(Width() - 1, y, z);
					if (GetLight(my_pos) > 0) {
						light_queue.emplace(this, my_pos);
					}
					if (other.GetLight(other_pos) > 0) {
						light_queue.emplace(&other, other_pos);
					}
				}
			}
			work_light();
		}
	} else if (other.position == position + Pos(1, 0, 0)) {
		if (neighbor[Block::FACE_RIGHT] != &other) {
			neighbor[Block::FACE_RIGHT] = &other;
			other.neighbor[Block::FACE_LEFT] = this;
			for (int z = 0; z < Depth(); ++z) {
				for (int y = 0; y < Height(); ++y) {
					Pos my_pos(Width() - 1, y, z);
					Pos other_pos(0, y, z);
					if (GetLight(my_pos) > 0) {
						light_queue.emplace(this, my_pos);
					}
					if (other.GetLight(other_pos) > 0) {
						light_queue.emplace(&other, other_pos);
					}
				}
			}
			work_light();
		}
	} else if (other.position == position + Pos(0, -1, 0)) {
		if (neighbor[Block::FACE_DOWN] != &other) {
			neighbor[Block::FACE_DOWN] = &other;
			other.neighbor[Block::FACE_UP] = this;
			for (int z = 0; z < Depth(); ++z) {
				for (int x = 0; x < Width(); ++x) {
					Pos my_pos(x, 0, z);
					Pos other_pos(x, Height() - 1, z);
					if (GetLight(my_pos) > 0) {
						light_queue.emplace(this, my_pos);
					}
					if (other.GetLight(other_pos) > 0) {
						light_queue.emplace(&other, other_pos);
					}
				}
			}
			work_light();
		}
	} else if (other.position == position + Pos(0, 1, 0)) {
		if (neighbor[Block::FACE_UP] != &other) {
			neighbor[Block::FACE_UP] = &other;
			other.neighbor[Block::FACE_DOWN] = this;
			for (int z = 0; z < Depth(); ++z) {
				for (int x = 0; x < Width(); ++x) {
					Pos my_pos(x, Height() - 1, z);
					Pos other_pos(x, 0, z);
					if (GetLight(my_pos) > 0) {
						light_queue.emplace(this, my_pos);
					}
					if (other.GetLight(other_pos) > 0) {
						light_queue.emplace(&other, other_pos);
					}
				}
			}
			work_light();
		}
	} else if (other.position == position + Pos(0, 0, -1)) {
		if (neighbor[Block::FACE_BACK] != &other) {
			neighbor[Block::FACE_BACK] = &other;
			other.neighbor[Block::FACE_FRONT] = this;
			for (int y = 0; y < Height(); ++y) {
				for (int x = 0; x < Width(); ++x) {
					Pos my_pos(x, y, 0);
					Pos other_pos(x, y, Depth() - 1);
					if (GetLight(my_pos) > 0) {
						light_queue.emplace(this, my_pos);
					}
					if (other.GetLight(other_pos) > 0) {
						light_queue.emplace(&other, other_pos);
					}
				}
			}
			work_light();
		}
	} else if (other.position == position + Pos(0, 0, 1)) {
		if (neighbor[Block::FACE_FRONT] != &other) {
			neighbor[Block::FACE_FRONT] = &other;
			other.neighbor[Block::FACE_BACK] = this;
			for (int y = 0; y < Height(); ++y) {
				for (int x = 0; x < Width(); ++x) {
					Pos my_pos(x, y, Depth() - 1);
					Pos other_pos(x, y, 0);
					if (GetLight(my_pos) > 0) {
						light_queue.emplace(this, my_pos);
					}
					if (other.GetLight(other_pos) > 0) {
						light_queue.emplace(&other, other_pos);
					}
				}
			}
			work_light();
		}
	}
}

void Chunk::ClearNeighbors() {
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		neighbor[i] = nullptr;
	}
}

void Chunk::Unlink() {
	for (int face = 0; face < Block::FACE_COUNT; ++face) {
		if (neighbor[face]) {
			neighbor[face]->neighbor[Block::Opposite(Block::Face(face))] = nullptr;
		}
	}
}

void Chunk::Relink() {
	for (int face = 0; face < Block::FACE_COUNT; ++face) {
		if (neighbor[face]) {
			neighbor[face]->neighbor[Block::Opposite(Block::Face(face))] = this;
		}
	}
}


void Chunk::SetLight(int index, int level) {
	if (light[index] != level) {
		light[index] = level;
		Invalidate();
	}
}

int Chunk::GetLight(int index) const {
	return light[index];
}

float Chunk::GetVertexLight(int index, const BlockModel::Position &vtx, const BlockModel::Normal &norm) const {
	float light = GetLight(index);
	Chunk::Pos pos(ToPos(index));

	Block::Face direct_face(Block::NormalFace(norm));
	// tis okay
	BlockLookup direct(const_cast<Chunk *>(this), pos, Block::NormalFace(norm));
	if (direct) {
		float direct_light = direct.GetLight();
		if (direct_light > light) {
			light = direct_light;
		}
	}

	// cheap alternative until AO etc are implemented
	// to tell the faces apart

	if (direct_face == Block::FACE_LEFT || direct_face == Block::FACE_RIGHT) {
		light -= 0.2;
	} else if (direct_face == Block::FACE_FRONT || direct_face == Block::FACE_BACK) {
		light -= 0.4;
	}

	return light;
}


bool Chunk::IsSurface(const Pos &pos) const {
	const Block &block = BlockAt(pos);
	if (!Type(block).visible) {
		return false;
	}
	for (int face = 0; face < Block::FACE_COUNT; ++face) {
		BlockLookup next = BlockLookup(const_cast<Chunk *>(this), pos, Block::Face(face));
		if (!next || !next.GetType().visible) {
			return true;
		}
	}
	return false;
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
	int &blkid,
	float &dist,
	glm::vec3 &normal
) const {
	// TODO: should be possible to heavily optimize this
	int id = 0;
	blkid = -1;
	dist = std::numeric_limits<float>::infinity();
	for (int z = 0; z < Depth(); ++z) {
		for (int y = 0; y < Height(); ++y) {
			for (int x = 0; x < Width(); ++x, ++id) {
				if (!Type(blocks[id]).visible) {
					continue;
				}
				float cur_dist;
				glm::vec3 cur_norm;
				if (Type(blocks[id]).shape->Intersects(ray, M * ToTransform(id), cur_dist, cur_norm)) {
					if (cur_dist < dist) {
						blkid = id;
						dist = cur_dist;
						normal = cur_norm;
					}
				}
			}
		}
	}

	if (blkid < 0) {
		return false;
	} else {
		normal = glm::vec3(BlockAt(blkid).Transform() * glm::vec4(normal, 0.0f));
		return true;
	}
}

void Chunk::Position(const Pos &pos) {
	position = pos;
}

glm::mat4 Chunk::Transform(const Pos &offset) const {
	return glm::translate((position - offset) * Extent());
}


namespace {

BlockModel::Buffer buf;

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

	BlockModel::Index vtx_counter = 0;
	for (size_t i = 0; i < Size(); ++i) {
		const BlockType &type = Type(blocks[i]);

		if (!type.visible || Obstructed(i)) continue;

		type.FillBlockModel(buf, ToTransform(i), vtx_counter);
		size_t vtx_begin = vtx_counter;
		vtx_counter += type.shape->VertexCount();

		for (size_t vtx = vtx_begin; vtx < vtx_counter; ++vtx) {
			buf.lights.emplace_back(GetVertexLight(i, buf.vertices[vtx], buf.normals[vtx]));
		}
	}

	model.Update(buf);
	dirty = false;
}

bool Chunk::Obstructed(int idx) const {
	Chunk::Pos pos(ToPos(idx));

	for (int f = 0; f < Block::FACE_COUNT; ++f) {
		Block::Face face = Block::Face(f);
		BlockLookup next(const_cast<Chunk *>(this), pos, face);
		if (!next || !next.GetType().FaceFilled(next.GetBlock(), Block::Opposite(face))) {
			return false;
		}
	}

	return true;
}

glm::mat4 Chunk::ToTransform(int idx) const {
	return glm::translate(glm::mat4(1.0f), ToCoords(idx)) * blocks[idx].Transform();
}


BlockLookup::BlockLookup(Chunk *c, const Chunk::Pos &p)
: chunk(c), pos(p) {
	while (pos.x >= Chunk::Width()) {
		if (chunk->HasNeighbor(Block::FACE_RIGHT)) {
			chunk = &chunk->GetNeighbor(Block::FACE_RIGHT);
			pos.x -= Chunk::Width();
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.x < 0) {
		if (chunk->HasNeighbor(Block::FACE_LEFT)) {
			chunk = &chunk->GetNeighbor(Block::FACE_LEFT);
			pos.x += Chunk::Width();
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.y >= Chunk::Height()) {
		if (chunk->HasNeighbor(Block::FACE_UP)) {
			chunk = &chunk->GetNeighbor(Block::FACE_UP);
			pos.y -= Chunk::Height();
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.y < 0) {
		if (chunk->HasNeighbor(Block::FACE_DOWN)) {
			chunk = &chunk->GetNeighbor(Block::FACE_DOWN);
			pos.y += Chunk::Height();
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.z >= Chunk::Depth()) {
		if (chunk->HasNeighbor(Block::FACE_FRONT)) {
			chunk = &chunk->GetNeighbor(Block::FACE_FRONT);
			pos.z -= Chunk::Depth();
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.z < 0) {
		if (chunk->HasNeighbor(Block::FACE_BACK)) {
			chunk = &chunk->GetNeighbor(Block::FACE_BACK);
			pos.z += Chunk::Depth();
		} else {
			chunk = nullptr;
			return;
		}
	}
}

BlockLookup::BlockLookup(Chunk *c, const Chunk::Pos &p, Block::Face face)
: chunk(c), pos(p) {
	pos += Block::FaceNormal(face);
	if (!Chunk::InBounds(pos)) {
		pos -= Block::FaceNormal(face) * Chunk::Extent();
		chunk = &chunk->GetNeighbor(face);
	}
}


ChunkLoader::ChunkLoader(const Config &config, const BlockTypeRegistry &reg, const Generator &gen)
: base(0, 0, 0)
, reg(reg)
, gen(gen)
, loaded()
, to_generate()
, to_free()
, load_dist(config.load_dist)
, unload_dist(config.unload_dist) {

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
				} else if (pos == base) {
					Generate(pos);

				//	light testing
				//	for (int i = 0; i < 16; ++i) {
				//		for (int j = 0; j < 16; ++j) {
				//			loaded.back().SetBlock(Chunk::Pos{  i, j,  0 }, Block(1));
				//			loaded.back().SetBlock(Chunk::Pos{  i, j, 15 }, Block(1));
				//			loaded.back().SetBlock(Chunk::Pos{  0, j,  i }, Block(1));
				//			loaded.back().SetBlock(Chunk::Pos{ 15, j,  i }, Block(1));
				//		}
				//	}
				//	loaded.back().SetBlock(Chunk::Pos{  1,  0,  1 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{ 14,  0,  1 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  1,  0, 14 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{ 14,  0, 14 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  1, 15,  1 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{ 14, 15,  1 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  1, 15, 14 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{ 14, 15, 14 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  7,  7,  0 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  8,  7,  0 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  7,  8,  0 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  8,  8,  0 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  7,  7, 15 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  8,  7, 15 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  7,  8, 15 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  8,  8, 15 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  0,  7,  7 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  0,  7,  8 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  0,  8,  7 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{  0,  8,  8 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{ 15,  7,  7 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{ 15,  7,  8 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{ 15,  8,  7 }, Block(13));
				//	loaded.back().SetBlock(Chunk::Pos{ 15,  8,  8 }, Block(13));
				//	loaded.back().Invalidate();
				//	loaded.back().CheckUpdate();

				//	orientation testing
				//	for (int i = 0; i < Block::FACE_COUNT; ++i) {
				//		for (int j = 0; j < Block::TURN_COUNT; ++j) {
				//			loaded.back().BlockAt(512 * j + 2 * i) = Block(3 * (j + 1), Block::Face(i), Block::Turn(j));
				//		}
				//	}
				//	loaded.back().Invalidate();
				//	loaded.back().CheckUpdate();
				} else {
					to_generate.emplace_back(pos);
				}
			}
		}
	}
	to_generate.sort(ChunkLess(base));
}

Chunk &ChunkLoader::Generate(const Chunk::Pos &pos) {
	loaded.emplace_back(reg);
	Chunk &chunk = loaded.back();
	chunk.Position(pos);
	gen(chunk);
	Insert(chunk);
	return chunk;
}

void ChunkLoader::Insert(Chunk &chunk) {
	for (Chunk &other : loaded) {
		chunk.SetNeighbor(other);
	}
}

void ChunkLoader::Remove(Chunk &chunk) {
	chunk.Unlink();
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

	return Generate(pos);
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
			Remove(*saved);
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
	GenerateSurrounding(base);
}

void ChunkLoader::GenerateSurrounding(const Chunk::Pos &pos) {
	const Chunk::Pos offset(load_dist, load_dist, load_dist);
	Generate(pos - offset, pos + offset);
}

void ChunkLoader::Update() {
	if (to_generate.empty()) {
		return;
	}

	Chunk::Pos pos(to_generate.front());
	to_generate.pop_front();

	for (auto iter(to_free.begin()), end(to_free.end()); iter != end; ++iter) {
		if (iter->Position() == pos) {
			iter->Relink();
			loaded.splice(loaded.end(), to_free, iter);
			return;
		}
	}

	if (to_free.empty()) {
		loaded.emplace_back(reg);
	} else {
		to_free.front().ClearNeighbors();
		loaded.splice(loaded.end(), to_free, to_free.begin());
	}
	Chunk &chunk = loaded.back();
	chunk.Position(pos);
	gen(chunk);
	Insert(chunk);
}

}
