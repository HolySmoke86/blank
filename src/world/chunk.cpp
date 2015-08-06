#include "BlockLookup.hpp"
#include "Chunk.hpp"
#include "ChunkLoader.hpp"

#include "Generator.hpp"
#include "WorldCollision.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <queue>


namespace blank {

constexpr int Chunk::width;
constexpr int Chunk::height;
constexpr int Chunk::depth;
constexpr int Chunk::size;


Chunk::Chunk(const BlockTypeRegistry &types) noexcept
: types(&types)
, neighbor{0}
, blocks{}
, light{0}
, model()
, position(0, 0, 0)
, dirty(false) {

}

Chunk::Chunk(Chunk &&other) noexcept
: types(other.types)
, model(std::move(other.model))
, position(other.position)
, dirty(other.dirty) {
	std::copy(other.neighbor, other.neighbor + sizeof(neighbor), neighbor);
	std::copy(other.blocks, other.blocks + sizeof(blocks), blocks);
	std::copy(other.light, other.light + sizeof(light), light);
}

Chunk &Chunk::operator =(Chunk &&other) noexcept {
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

	int Get() const noexcept { return chunk->GetLight(pos); }
	void Set(int level) noexcept { chunk->SetLight(pos, level); }

	const BlockType &GetType() const noexcept { return chunk->Type(Chunk::ToIndex(pos)); }

	bool HasNext(Block::Face face) noexcept {
		const BlockType &type = GetType();
		if (type.block_light && !type.luminosity) return false;
		const BlockLookup next(chunk, pos, face);
		return next;
	}
	SetNode GetNext(Block::Face face) noexcept {
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


	bool HasNext(Block::Face face) noexcept {
		const BlockLookup next(chunk, pos, face);
		return next;
	}
	UnsetNode GetNext(Block::Face face) noexcept { return UnsetNode(SetNode::GetNext(face)); }

};

std::queue<SetNode> light_queue;
std::queue<UnsetNode> dark_queue;

void work_light() noexcept {
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

void work_dark() noexcept {
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

void Chunk::SetBlock(int index, const Block &block) noexcept {
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
		Pos pos(ToPos(index));
		for (int face = 0; face < Block::FACE_COUNT; ++face) {
			BlockLookup next_block(this, pos, Block::Face(face));
			if (next_block) {
				level = std::max(level, next_block.GetLight());
			}
		}
		if (level > 1) {
			SetLight(index, level - 1);
			light_queue.emplace(this, pos);
			work_light();
		}
	}
}

namespace {

// propagate light from a's edge to b
void edge_light(
	Chunk &a, const Chunk::Pos &a_pos,
	Chunk &b, const Chunk::Pos &b_pos
) noexcept {
	if (a.GetLight(a_pos) > 1) {
		const BlockType &b_type = b.Type(Chunk::ToIndex(b_pos));
		if (!b_type.block_light) {
			light_queue.emplace(&a, a_pos);
		}
		if (b_type.visible) {
			b.Invalidate();
		}
	}
}

}

void Chunk::SetNeighbor(Chunk &other) noexcept {
	if (other.position == position + Pos(-1, 0, 0)) {
		if (neighbor[Block::FACE_LEFT] != &other) {
			neighbor[Block::FACE_LEFT] = &other;
			other.neighbor[Block::FACE_RIGHT] = this;
			for (int z = 0; z < depth; ++z) {
				for (int y = 0; y < height; ++y) {
					Pos my_pos(0, y, z);
					Pos other_pos(width - 1, y, z);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			work_light();
		}
	} else if (other.position == position + Pos(1, 0, 0)) {
		if (neighbor[Block::FACE_RIGHT] != &other) {
			neighbor[Block::FACE_RIGHT] = &other;
			other.neighbor[Block::FACE_LEFT] = this;
			for (int z = 0; z < depth; ++z) {
				for (int y = 0; y < height; ++y) {
					Pos my_pos(width - 1, y, z);
					Pos other_pos(0, y, z);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			work_light();
		}
	} else if (other.position == position + Pos(0, -1, 0)) {
		if (neighbor[Block::FACE_DOWN] != &other) {
			neighbor[Block::FACE_DOWN] = &other;
			other.neighbor[Block::FACE_UP] = this;
			for (int z = 0; z < depth; ++z) {
				for (int x = 0; x < width; ++x) {
					Pos my_pos(x, 0, z);
					Pos other_pos(x, height - 1, z);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			work_light();
		}
	} else if (other.position == position + Pos(0, 1, 0)) {
		if (neighbor[Block::FACE_UP] != &other) {
			neighbor[Block::FACE_UP] = &other;
			other.neighbor[Block::FACE_DOWN] = this;
			for (int z = 0; z < depth; ++z) {
				for (int x = 0; x < width; ++x) {
					Pos my_pos(x, height - 1, z);
					Pos other_pos(x, 0, z);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			work_light();
		}
	} else if (other.position == position + Pos(0, 0, -1)) {
		if (neighbor[Block::FACE_BACK] != &other) {
			neighbor[Block::FACE_BACK] = &other;
			other.neighbor[Block::FACE_FRONT] = this;
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					Pos my_pos(x, y, 0);
					Pos other_pos(x, y, depth - 1);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			work_light();
		}
	} else if (other.position == position + Pos(0, 0, 1)) {
		if (neighbor[Block::FACE_FRONT] != &other) {
			neighbor[Block::FACE_FRONT] = &other;
			other.neighbor[Block::FACE_BACK] = this;
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					Pos my_pos(x, y, depth - 1);
					Pos other_pos(x, y, 0);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			work_light();
		}
	}
}

void Chunk::ClearNeighbors() noexcept {
	for (int face = 0; face < Block::FACE_COUNT; ++face) {
		if (neighbor[face]) {
			neighbor[face]->neighbor[Block::Opposite(Block::Face(face))] = nullptr;
			neighbor[face] = nullptr;
		}
	}
}


void Chunk::SetLight(int index, int level) noexcept {
	if (light[index] != level) {
		light[index] = level;
		Invalidate();
	}
}

int Chunk::GetLight(int index) const noexcept {
	return light[index];
}

float Chunk::GetVertexLight(const Pos &pos, const BlockModel::Position &vtx, const EntityModel::Normal &norm) const noexcept {
	int index = ToIndex(pos);
	float light = GetLight(index);

	Block::Face direct_face(Block::NormalFace(norm));
	// tis okay
	BlockLookup direct(const_cast<Chunk *>(this), pos, Block::NormalFace(norm));
	if (direct) {
		float direct_light = direct.GetLight();
		if (direct_light > light) {
			light = direct_light;
		}
	} else {
		return light;
	}

	if (Type(BlockAt(index)).luminosity > 0 || direct.GetType().block_light) {
		return light;
	}

	Block::Face edge[2];
	switch (Block::Axis(direct_face)) {
		case 0: // X
			edge[0] = (vtx.y - pos.y) > 0.5f ? Block::FACE_UP : Block::FACE_DOWN;
			edge[1] = (vtx.z - pos.z) > 0.5f ? Block::FACE_FRONT : Block::FACE_BACK;
			break;
		case 1: // Y
			edge[0] = (vtx.z - pos.z) > 0.5f ? Block::FACE_FRONT : Block::FACE_BACK;
			edge[1] = (vtx.x - pos.x) > 0.5f ? Block::FACE_RIGHT : Block::FACE_LEFT;
			break;
		case 2: // Z
			edge[0] = (vtx.x - pos.x) > 0.5f ? Block::FACE_RIGHT : Block::FACE_LEFT;
			edge[1] = (vtx.y - pos.y) > 0.5f ? Block::FACE_UP : Block::FACE_DOWN;
			break;
	}

	int num = 1;
	int occlusion = 0;

	BlockLookup next[2] = {
		direct.Next(edge[0]),
		direct.Next(edge[1]),
	};

	if (next[0]) {
		if (next[0].GetType().block_light) {
			++occlusion;
		} else {
			light += next[0].GetLight();
			++num;
		}
	}
	if (next[1]) {
		if (next[1].GetType().block_light) {
			++occlusion;
		} else {
			light += next[1].GetLight();
			++num;
		}
	}
	if (occlusion < 2) {
		if (next[0]) {
			BlockLookup corner = next[0].Next(edge[1]);
			if (corner) {
				if (corner.GetType().block_light) {
					++occlusion;
				} else {
					light += corner.GetLight();
					++num;
				}
			}
		} else if (next[1]) {
			BlockLookup corner = next[1].Next(edge[0]);
			if (corner) {
				if (corner.GetType().block_light) {
					++occlusion;
				} else {
					light += corner.GetLight();
					++num;
				}
			}
		}
	} else {
		++occlusion;
	}

	return (light / num) - (occlusion * 0.8f);
}


bool Chunk::IsSurface(const Pos &pos) const noexcept {
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


void Chunk::Draw() noexcept {
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
) const noexcept {
	int idx = 0;
	blkid = -1;
	dist = std::numeric_limits<float>::infinity();
	for (int z = 0; z < depth; ++z) {
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x, ++idx) {
				const BlockType &type = Type(idx);
				if (!type.visible) {
					continue;
				}
				float cur_dist;
				glm::vec3 cur_norm;
				if (type.shape->Intersects(ray, M * ToTransform(Pos(x, y, z), idx), cur_dist, cur_norm)) {
					if (cur_dist < dist) {
						blkid = idx;
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

bool Chunk::Intersection(
	const AABB &box,
	const glm::mat4 &Mbox,
	const glm::mat4 &Mchunk,
	std::vector<WorldCollision> &col
) const noexcept {
	bool any = false;
	float penetration;
	glm::vec3 normal;

	if (!blank::Intersection(box, Mbox, Bounds(), Mchunk, penetration, normal)) {
		return false;
	}
	for (int idx = 0, z = 0; z < depth; ++z) {
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x, ++idx) {
				const BlockType &type = Type(idx);
				if (!type.collision) {
					continue;
				}
				if (type.shape->Intersects(Mchunk * ToTransform(Pos(x, y, z), idx), box, Mbox, penetration, normal)) {
					col.emplace_back(this, idx, penetration, normal);
					any = true;
				}
			}
		}
	}
	return any;
}


namespace {

BlockModel::Buffer buf;

}

void Chunk::CheckUpdate() noexcept {
	if (dirty) {
		Update();
	}
}

void Chunk::Update() noexcept {
	int vtx_count = 0, idx_count = 0;
	for (const auto &block : blocks) {
		const Shape *shape = Type(block).shape;
		vtx_count += shape->VertexCount();
		idx_count += shape->VertexIndexCount();
	}
	buf.Clear();
	buf.Reserve(vtx_count, idx_count);

	int idx = 0;
	BlockModel::Index vtx_counter = 0;
	for (size_t z = 0; z < depth; ++z) {
		for (size_t y = 0; y < height; ++y) {
			for (size_t x = 0; x < width; ++x, ++idx) {
				const BlockType &type = Type(BlockAt(idx));
				const Pos pos(x, y, z);

				if (!type.visible || Obstructed(pos).All()) continue;

				type.FillBlockModel(buf, ToTransform(pos, idx), vtx_counter);
				size_t vtx_begin = vtx_counter;
				vtx_counter += type.shape->VertexCount();

				for (size_t vtx = vtx_begin; vtx < vtx_counter; ++vtx) {
					buf.lights.emplace_back(GetVertexLight(
						pos,
						buf.vertices[vtx],
						type.shape->VertexNormal(vtx - vtx_begin, BlockAt(idx).Transform())
					));
				}
			}
		}
	}

	model.Update(buf);
	dirty = false;
}

Block::FaceSet Chunk::Obstructed(const Pos &pos) const noexcept {
	Block::FaceSet result;

	for (int f = 0; f < Block::FACE_COUNT; ++f) {
		Block::Face face = Block::Face(f);
		BlockLookup next(const_cast<Chunk *>(this), pos, face);
		if (next && next.GetType().FaceFilled(next.GetBlock(), Block::Opposite(face))) {
			result.Set(face);
		}
	}

	return result;
}

glm::mat4 Chunk::ToTransform(const Pos &pos, int idx) const noexcept {
	return glm::translate(ToCoords(pos)) * BlockAt(idx).Transform();
}


BlockLookup::BlockLookup(Chunk *c, const Chunk::Pos &p) noexcept
: chunk(c), pos(p) {
	while (pos.x >= Chunk::width) {
		if (chunk->HasNeighbor(Block::FACE_RIGHT)) {
			chunk = &chunk->GetNeighbor(Block::FACE_RIGHT);
			pos.x -= Chunk::width;
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.x < 0) {
		if (chunk->HasNeighbor(Block::FACE_LEFT)) {
			chunk = &chunk->GetNeighbor(Block::FACE_LEFT);
			pos.x += Chunk::width;
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.y >= Chunk::height) {
		if (chunk->HasNeighbor(Block::FACE_UP)) {
			chunk = &chunk->GetNeighbor(Block::FACE_UP);
			pos.y -= Chunk::height;
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.y < 0) {
		if (chunk->HasNeighbor(Block::FACE_DOWN)) {
			chunk = &chunk->GetNeighbor(Block::FACE_DOWN);
			pos.y += Chunk::height;
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.z >= Chunk::depth) {
		if (chunk->HasNeighbor(Block::FACE_FRONT)) {
			chunk = &chunk->GetNeighbor(Block::FACE_FRONT);
			pos.z -= Chunk::depth;
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.z < 0) {
		if (chunk->HasNeighbor(Block::FACE_BACK)) {
			chunk = &chunk->GetNeighbor(Block::FACE_BACK);
			pos.z += Chunk::depth;
		} else {
			chunk = nullptr;
			return;
		}
	}
}

BlockLookup::BlockLookup(Chunk *c, const Chunk::Pos &p, Block::Face face) noexcept
: chunk(c), pos(p) {
	pos += Block::FaceNormal(face);
	if (!Chunk::InBounds(pos)) {
		pos -= Block::FaceNormal(face) * Chunk::Extent();
		chunk = &chunk->GetNeighbor(face);
	}
}


ChunkLoader::ChunkLoader(const Config &config, const BlockTypeRegistry &reg, const Generator &gen) noexcept
: base(0, 0, 0)
, reg(reg)
, gen(gen)
, loaded()
, to_generate()
, to_free()
, gen_timer(config.gen_limit)
, load_dist(config.load_dist)
, unload_dist(config.unload_dist) {
	gen_timer.Start();
}

namespace {

struct ChunkLess {

	explicit ChunkLess(const Chunk::Pos &base) noexcept
	: base(base) { }

	bool operator ()(const Chunk::Pos &a, const Chunk::Pos &b) const noexcept {
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

void ChunkLoader::Insert(Chunk &chunk) noexcept {
	for (Chunk &other : loaded) {
		chunk.SetNeighbor(other);
	}
}

std::list<Chunk>::iterator ChunkLoader::Remove(std::list<Chunk>::iterator chunk) noexcept {
	// fetch next entry while chunk's still in the list
	std::list<Chunk>::iterator next = chunk;
	++next;
	// unlink neighbors so they won't reference a dead chunk
	chunk->ClearNeighbors();
	// and move it from loaded to free list
	to_free.splice(to_free.end(), loaded, chunk);
	return next;
}

Chunk *ChunkLoader::Loaded(const Chunk::Pos &pos) noexcept {
	for (Chunk &chunk : loaded) {
		if (chunk.Position() == pos) {
			return &chunk;
		}
	}
	return nullptr;
}

bool ChunkLoader::Queued(const Chunk::Pos &pos) noexcept {
	for (const Chunk::Pos &chunk : to_generate) {
		if (chunk == pos) {
			return true;
		}
	}
	return false;
}

bool ChunkLoader::Known(const Chunk::Pos &pos) noexcept {
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

bool ChunkLoader::OutOfRange(const Chunk::Pos &pos) const noexcept {
	return std::abs(base.x - pos.x) > unload_dist
			|| std::abs(base.y - pos.y) > unload_dist
			|| std::abs(base.z - pos.z) > unload_dist;
}

void ChunkLoader::Rebase(const Chunk::Pos &new_base) {
	if (new_base == base) {
		return;
	}
	base = new_base;

	// unload far away chunks
	for (auto iter(loaded.begin()), end(loaded.end()); iter != end;) {
		if (OutOfRange(*iter)) {
			iter = Remove(iter);
		} else {
			++iter;
		}
	}
	// abort far away queued chunks
	for (auto iter(to_generate.begin()), end(to_generate.end()); iter != end;) {
		if (OutOfRange(*iter)) {
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

void ChunkLoader::Update(int dt) {
	// check if a chunk generation is scheduled for this frame
	// and if there's a chunk waiting to be generated
	gen_timer.Update(dt);
	if (gen_timer.Hit()) {
		LoadOne();
	}
}

void ChunkLoader::LoadN(std::size_t n) {
	std::size_t end = std::min(n, ToLoad());
	for (std::size_t i = 0; i < end; ++i) {
		LoadOne();
	}
}

void ChunkLoader::LoadOne() {
	if (to_generate.empty()) return;

	// take position of next chunk in queue
	Chunk::Pos pos(to_generate.front());
	to_generate.pop_front();

	// look if the same chunk was already generated and still lingering
	for (auto iter(to_free.begin()), end(to_free.end()); iter != end; ++iter) {
		if (iter->Position() == pos) {
			loaded.splice(loaded.end(), to_free, iter);
			Insert(loaded.back());
			return;
		}
	}

	// if the free list is empty, allocate a new chunk
	// otherwise clear an unused one
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
