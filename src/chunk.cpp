#include "chunk.hpp"

#include "generator.hpp"

#include <limits>
#include <queue>
#include <glm/gtx/transform.hpp>


namespace blank {

Chunk::Chunk(const BlockTypeRegistry &types)
: types(&types)
, neighbor{ 0, 0, 0, 0, 0, 0 }
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
	for (size_t i = 0; i < Block::FACE_COUNT; ++i) {
		neighbor[i] = other.neighbor[i];
	}
}

Chunk &Chunk::operator =(Chunk &&other) {
	types = other.types;
	for (size_t i = 0; i < Block::FACE_COUNT; ++i) {
		neighbor[i] = other.neighbor[i];
	}
	blocks = std::move(other.blocks);
	model = std::move(other.model);
	dirty = other.dirty;
	return *this;
}


void Chunk::SetNeighbor(Chunk &other) {
	if (other.position == position - Pos(-1, 0, 0)) {
		neighbor[Block::FACE_LEFT] = &other;
		other.neighbor[Block::FACE_RIGHT] = this;
	} else if (other.position == position - Pos(1, 0, 0)) {
		neighbor[Block::FACE_RIGHT] = &other;
		other.neighbor[Block::FACE_LEFT] = this;
	} else if (other.position == position - Pos(0, -1, 0)) {
		neighbor[Block::FACE_DOWN] = &other;
		other.neighbor[Block::FACE_UP] = this;
	} else if (other.position == position - Pos(0, 1, 0)) {
		neighbor[Block::FACE_UP] = &other;
		other.neighbor[Block::FACE_DOWN] = this;
	} else if (other.position == position - Pos(0, 0, -1)) {
		neighbor[Block::FACE_BACK] = &other;
		other.neighbor[Block::FACE_FRONT] = this;
	} else if (other.position == position - Pos(0, 0, 1)) {
		neighbor[Block::FACE_FRONT] = &other;
		other.neighbor[Block::FACE_BACK] = this;
	}
}

void Chunk::ClearNeighbors() {
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		neighbor[i] = nullptr;
	}
}

void Chunk::Unlink() {
	if (neighbor[Block::FACE_UP]) {
		neighbor[Block::FACE_UP]->neighbor[Block::FACE_DOWN] = nullptr;
	}
	if (neighbor[Block::FACE_DOWN]) {
		neighbor[Block::FACE_DOWN]->neighbor[Block::FACE_UP] = nullptr;
	}
	if (neighbor[Block::FACE_LEFT]) {
		neighbor[Block::FACE_LEFT]->neighbor[Block::FACE_RIGHT] = nullptr;
	}
	if (neighbor[Block::FACE_RIGHT]) {
		neighbor[Block::FACE_RIGHT]->neighbor[Block::FACE_LEFT] = nullptr;
	}
	if (neighbor[Block::FACE_FRONT]) {
		neighbor[Block::FACE_FRONT]->neighbor[Block::FACE_BACK] = nullptr;
	}
	if (neighbor[Block::FACE_BACK]) {
		neighbor[Block::FACE_BACK]->neighbor[Block::FACE_FRONT] = nullptr;
	}
}

void Chunk::Relink() {
	if (neighbor[Block::FACE_UP]) {
		neighbor[Block::FACE_UP]->neighbor[Block::FACE_DOWN] = this;
	}
	if (neighbor[Block::FACE_DOWN]) {
		neighbor[Block::FACE_DOWN]->neighbor[Block::FACE_UP] = this;
	}
	if (neighbor[Block::FACE_LEFT]) {
		neighbor[Block::FACE_LEFT]->neighbor[Block::FACE_RIGHT] = this;
	}
	if (neighbor[Block::FACE_RIGHT]) {
		neighbor[Block::FACE_RIGHT]->neighbor[Block::FACE_LEFT] = this;
	}
	if (neighbor[Block::FACE_FRONT]) {
		neighbor[Block::FACE_FRONT]->neighbor[Block::FACE_BACK] = this;
	}
	if (neighbor[Block::FACE_BACK]) {
		neighbor[Block::FACE_BACK]->neighbor[Block::FACE_FRONT] = this;
	}
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
		const Block *next = chunk->FindNext(pos, face);
		return next && !chunk->Type(*next).block_light;
	}
	SetNode GetNext(Block::Face face) {
		Chunk::Pos next_pos(pos + Block::FaceNormal(face));
		if (Chunk::InBounds(next_pos)) {
			return SetNode(chunk, next_pos);
		} else {
			return SetNode(&chunk->GetNeighbor(face), next_pos - (Block::FaceNormal(face) * Chunk::Extent()));
		}
	}

};

struct UnsetNode
: public SetNode {

	int level;

	UnsetNode(Chunk *chunk, Chunk::Pos pos)
	: SetNode(chunk, pos), level(Get()) { }

	UnsetNode(const SetNode &set)
	: SetNode(set), level(Get()) { }

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
	work_light();
}

}

void Chunk::SetBlock(int index, const Block &block) {
	const BlockType &old_type = Type(blocks[index]);
	const BlockType &new_type = Type(block);

	blocks[index] = block;

	if (&old_type == &new_type) return;

	if (new_type.luminosity > 0) {
		if (GetLight(index) < new_type.luminosity) {
			SetLight(index, new_type.luminosity);
			light_queue.emplace(this, ToPos(index));
			work_light();
		}
	} else if (new_type.block_light && GetLight(index) != 0) {
		SetLight(index, 0);
		dark_queue.emplace(this, ToPos(index));
		work_dark();
	} else if (old_type.block_light && !new_type.block_light) {
		int level = 0;
		for (int face = 0; face < Block::FACE_COUNT; ++face) {
			Pos next_pos(ToPos(index) + Block::FaceNormal(Block::Face(face)));
			int next_level = 0;
			if (InBounds(next_pos)) {
				next_level = GetLight(next_pos);
			} else {
				if (HasNeighbor(Block::Face(face))) {
					next_pos -= (Block::FaceNormal(Block::Face(face)) * Chunk::Extent());
					next_level = GetNeighbor(Block::Face(face)).GetLight(next_pos);
				}
			}
			if (level < next_level) {
				level = next_level;
			}
		}
		if (level > 1) {
			SetLight(index, level - 1);
			light_queue.emplace(this, ToPos(index));
			work_light();
		}
	}
}

const Block *Chunk::FindNext(const Pos &pos, Block::Face face) const {
	Pos next_pos(pos + Block::FaceNormal(face));
	if (InBounds(next_pos)) {
		return &BlockAt(pos + Block::FaceNormal(face));
	} else if (HasNeighbor(face)) {
		return &GetNeighbor(face).BlockAt(next_pos - (Block::FaceNormal(face) * Extent()));
	} else {
		return nullptr;
	}
}


void Chunk::SetLight(int index, int level) {
	light[index] = level;
}

int Chunk::GetLight(int index) const {
	return light[index];
}


bool Chunk::IsSurface(const Pos &pos) const {
	const Block &block = BlockAt(pos);
	if (!Type(block).visible) {
		return false;
	}
	for (int face = 0; face < Block::FACE_COUNT; ++face) {
		const Block *next = FindNext(pos, Block::Face(face));
		if (!next || !Type(*next).visible) {
			return true;
		}
	}
	return false;
}


void Chunk::Allocate() {
	blocks.resize(Size(), Block(0));
	light.resize(Size(), 0);
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

Model::Buffer buf;

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

		if (!type.visible || Obstructed(i)) continue;

		type.FillModel(buf, ToTransform(i), vtx_counter);
		vtx_counter += type.shape->VertexCount();
	}

	model.Update(buf);
	dirty = false;
}

bool Chunk::Obstructed(int idx) const {
	Chunk::Pos pos(ToPos(idx));

	Chunk::Pos left_pos(pos + Chunk::Pos(-1, 0, 0));
	const Block *left_block = nullptr;
	if (InBounds(left_pos)) {
		left_block = &BlockAt(left_pos);
	} else if (HasNeighbor(Block::FACE_LEFT)) {
		left_pos += Chunk::Pos(Width(), 0, 0);
		left_block = &GetNeighbor(Block::FACE_LEFT).BlockAt(left_pos);
	} else {
		return false;
	}
	if (!Type(*left_block).FaceFilled(*left_block, Block::FACE_RIGHT)) {
		return false;
	}

	Chunk::Pos right_pos(pos + Chunk::Pos(1, 0, 0));
	const Block *right_block = nullptr;
	if (InBounds(right_pos)) {
		right_block = &BlockAt(right_pos);
	} else if (HasNeighbor(Block::FACE_RIGHT)) {
		right_pos += Chunk::Pos(-Width(), 0, 0);
		right_block = &GetNeighbor(Block::FACE_RIGHT).BlockAt(right_pos);
	} else {
		return false;
	}
	if (!Type(*right_block).FaceFilled(*right_block, Block::FACE_LEFT)) {
		return false;
	}

	Chunk::Pos down_pos(pos + Chunk::Pos(0, -1, 0));
	const Block *down_block = nullptr;
	if (InBounds(down_pos)) {
		down_block = &BlockAt(down_pos);
	} else if (HasNeighbor(Block::FACE_DOWN)) {
		down_pos += Chunk::Pos(0, Height(), 0);
		down_block = &GetNeighbor(Block::FACE_DOWN).BlockAt(down_pos);
	} else {
		return false;
	}
	if (!Type(*down_block).FaceFilled(*down_block, Block::FACE_UP)) {
		return false;
	}

	Chunk::Pos up_pos(pos + Chunk::Pos(0, 1, 0));
	const Block *up_block = nullptr;
	if (InBounds(up_pos)) {
		up_block = &BlockAt(up_pos);
	} else if (HasNeighbor(Block::FACE_UP)) {
		up_pos += Chunk::Pos(0, -Height(), 0);
		up_block = &GetNeighbor(Block::FACE_UP).BlockAt(up_pos);
	} else {
		return false;
	}
	if (!Type(*up_block).FaceFilled(*up_block, Block::FACE_DOWN)) {
		return false;
	}

	Chunk::Pos back_pos(pos + Chunk::Pos(0, 0, -1));
	const Block *back_block = nullptr;
	if (InBounds(back_pos)) {
		back_block = &BlockAt(back_pos);
	} else if (HasNeighbor(Block::FACE_BACK)) {
		back_pos += Chunk::Pos(0, 0, Depth());
		back_block = &GetNeighbor(Block::FACE_BACK).BlockAt(back_pos);
	} else {
		return false;
	}
	if (!Type(*back_block).FaceFilled(*back_block, Block::FACE_FRONT)) {
		return false;
	}

	Chunk::Pos front_pos(pos + Chunk::Pos(0, 0, 1));
	const Block *front_block = nullptr;
	if (InBounds(front_pos)) {
		front_block = &BlockAt(front_pos);
	} else if (HasNeighbor(Block::FACE_FRONT)) {
		front_pos += Chunk::Pos(0, 0, -Depth());
		front_block = &GetNeighbor(Block::FACE_FRONT).BlockAt(front_pos);
	} else {
		return false;
	}
	if (!Type(*front_block).FaceFilled(*front_block, Block::FACE_BACK)) {
		return false;
	}

	return true;
}

glm::mat4 Chunk::ToTransform(int idx) const {
	return glm::translate(glm::mat4(1.0f), ToCoords(idx)) * blocks[idx].Transform();
}


ChunkLoader::ChunkLoader(const BlockTypeRegistry &reg, const Generator &gen)
: base(0, 0, 0)
, reg(reg)
, gen(gen)
, loaded()
, to_generate()
, to_free()
, load_dist(6)
, unload_dist(8) {

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
	Insert(chunk);
	gen(chunk);
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
	const Chunk::Pos offset(load_dist, load_dist, load_dist);
	Generate(base - offset, base + offset);
}

void ChunkLoader::Update() {
	bool reused = false;
	if (!to_generate.empty()) {
		Chunk::Pos pos(to_generate.front());

		for (auto iter(to_free.begin()), end(to_free.end()); iter != end; ++iter) {
			if (iter->Position() == pos) {
				iter->Relink();
				loaded.splice(loaded.end(), to_free, iter);
				reused = true;
				break;
			}
		}

		if (!reused) {
			if (to_free.empty()) {
				loaded.emplace_back(reg);
			} else {
				to_free.front().ClearNeighbors();
				loaded.splice(loaded.end(), to_free, to_free.begin());
				reused = true;
			}
			Chunk &chunk = loaded.back();
			chunk.Position(pos);
			Insert(chunk);
			gen(chunk);
		}
		to_generate.pop_front();
	}

	if (!reused && !to_free.empty()) {
		to_free.pop_front();
	}
}

}
