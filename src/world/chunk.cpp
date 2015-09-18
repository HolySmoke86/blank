#include "BlockLookup.hpp"
#include "Chunk.hpp"
#include "ChunkIndex.hpp"
#include "ChunkLoader.hpp"
#include "ChunkRenderer.hpp"
#include "ChunkStore.hpp"

#include "Generator.hpp"
#include "WorldCollision.hpp"
#include "../app/Assets.hpp"
#include "../graphics/BlockLighting.hpp"
#include "../graphics/Viewport.hpp"
#include "../io/WorldSave.hpp"
#include "../model/BlockModel.hpp"

#include <algorithm>
#include <limits>
#include <ostream>
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
, position(0, 0, 0)
, ref_count(0)
, dirty_model(false)
, dirty_save(false) {

}

Chunk::Chunk(Chunk &&other) noexcept
: types(other.types)
, position(other.position)
, dirty_model(other.dirty_model)
, dirty_save(other.dirty_save) {
	std::copy(other.neighbor, other.neighbor + sizeof(neighbor), neighbor);
	std::copy(other.blocks, other.blocks + sizeof(blocks), blocks);
	std::copy(other.light, other.light + sizeof(light), light);
}

Chunk &Chunk::operator =(Chunk &&other) noexcept {
	types = other.types;
	std::copy(other.neighbor, other.neighbor + sizeof(neighbor), neighbor);
	std::copy(other.blocks, other.blocks + sizeof(blocks), blocks);
	std::copy(other.light, other.light + sizeof(light), light);
	position = other.position;
	dirty_model = other.dirty_save;
	dirty_save = other.dirty_save;
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
	Invalidate();

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

void Chunk::SetNeighbor(Block::Face face, Chunk &other) noexcept {
	neighbor[face] = &other;
	other.neighbor[Block::Opposite(face)] = this;

	switch (face) {

		default:
			// error
			break;

		case Block::FACE_LEFT:
			for (int z = 0; z < depth; ++z) {
				for (int y = 0; y < height; ++y) {
					Pos my_pos(0, y, z);
					Pos other_pos(width - 1, y, z);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			break;

		case Block::FACE_RIGHT:
			for (int z = 0; z < depth; ++z) {
				for (int y = 0; y < height; ++y) {
					Pos my_pos(width - 1, y, z);
					Pos other_pos(0, y, z);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			break;

		case Block::FACE_DOWN:
			for (int z = 0; z < depth; ++z) {
				for (int x = 0; x < width; ++x) {
					Pos my_pos(x, 0, z);
					Pos other_pos(x, height - 1, z);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			break;

		case Block::FACE_UP:
			for (int z = 0; z < depth; ++z) {
				for (int x = 0; x < width; ++x) {
					Pos my_pos(x, height - 1, z);
					Pos other_pos(x, 0, z);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			break;

		case Block::FACE_BACK:
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					Pos my_pos(x, y, 0);
					Pos other_pos(x, y, depth - 1);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			break;

		case Block::FACE_FRONT:
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					Pos my_pos(x, y, depth - 1);
					Pos other_pos(x, y, 0);
					edge_light(*this, my_pos, other, other_pos);
					edge_light(other, other_pos, *this, my_pos);
				}
			}
			break;

	}
	work_light();
}

void Chunk::Unlink() noexcept {
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


bool Chunk::Intersection(
	const Ray &ray,
	const glm::mat4 &M,
	WorldCollision &coll
) noexcept {
	int idx = 0;
	coll.chunk = this;
	coll.block = -1;
	coll.depth = std::numeric_limits<float>::infinity();
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
					if (cur_dist < coll.depth) {
						coll.block = idx;
						coll.depth = cur_dist;
						coll.normal = cur_norm;
					}
				}
			}
		}
	}

	if (coll.block < 0) {
		return false;
	} else {
		coll.normal = glm::vec3(BlockAt(coll.block).Transform() * glm::vec4(coll.normal, 0.0f));
		return true;
	}
}

bool Chunk::Intersection(
	const AABB &box,
	const glm::mat4 &Mbox,
	const glm::mat4 &Mchunk,
	std::vector<WorldCollision> &col
) noexcept {
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

void Chunk::Update(BlockModel &model) noexcept {
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
	ClearModel();
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


ChunkLoader::ChunkLoader(
	ChunkStore &store,
	const Generator &gen,
	const WorldSave &save
) noexcept
: store(store)
, gen(gen)
, save(save) {

}

void ChunkLoader::Update(int dt) {
	// check if there's chunks waiting to be loaded
	// load until one of load or generation limits was hit
	constexpr int max_load = 10;
	constexpr int max_gen = 1;
	int loaded = 0;
	int generated = 0;
	while (loaded < max_load && generated < max_gen && store.HasMissing()) {
		if (LoadOne()) {
			++generated;
		} else {
			++loaded;
		}
	}

	// store a few chunks as well
	constexpr int max_save = 10;
	int saved = 0;
	for (Chunk &chunk : store) {
		if (chunk.ShouldUpdateSave()) {
			save.Write(chunk);
			++saved;
			if (saved >= max_save) {
				break;
			}
		}
	}
}

int ChunkLoader::ToLoad() const noexcept {
	return store.EstimateMissing();
}

bool ChunkLoader::LoadOne() {
	if (!store.HasMissing()) return false;

	Chunk::Pos pos = store.NextMissing();
	Chunk *chunk = store.Allocate(pos);
	if (!chunk) {
		// chunk store corrupted?
		return false;
	}

	if (save.Exists(pos)) {
		save.Read(*chunk);
		return false;
	} else {
		gen(*chunk);
		return true;
	}
}

void ChunkLoader::LoadN(std::size_t n) {
	std::size_t end = std::min(n, std::size_t(ToLoad()));
	for (std::size_t i = 0; i < end && store.HasMissing(); ++i) {
		LoadOne();
	}
}


ChunkRenderer::ChunkRenderer(ChunkIndex &index)
: index(index)
, models(index.TotalChunks())
, block_tex()
, fog_density(0.0f) {

}

ChunkRenderer::~ChunkRenderer() {

}

int ChunkRenderer::MissingChunks() const noexcept {
	return index.MissingChunks();
}

void ChunkRenderer::LoadTextures(const AssetLoader &loader, const TextureIndex &tex_index) {
	block_tex.Bind();
	loader.LoadTextures(tex_index, block_tex);
	block_tex.FilterNearest();
}

void ChunkRenderer::Update(int dt) {
	for (int i = 0, updates = 0; updates < dt && i < index.TotalChunks(); ++i) {
		if (index[i] && index[i]->ShouldUpdateModel()) {
			index[i]->Update(models[i]);
			++updates;
		}
	}
}

void ChunkRenderer::Render(Viewport &viewport) {
	BlockLighting &chunk_prog = viewport.ChunkProgram();
	chunk_prog.SetTexture(block_tex);
	chunk_prog.SetFogDensity(fog_density);

	for (int i = 0; i < index.TotalChunks(); ++i) {
		if (!index[i]) continue;
		glm::mat4 m(index[i]->Transform(index.Base()));
		glm::mat4 mvp(chunk_prog.GetVP() * m);
		if (!CullTest(Chunk::Bounds(), mvp)) {
			if (index[i]->ShouldUpdateModel()) {
				index[i]->Update(models[i]);
			}
			chunk_prog.SetM(m);
			models[i].Draw();
		}
	}
}


ChunkIndex::ChunkIndex(ChunkStore &store, const Chunk::Pos &base, int extent)
: store(store)
, base(base)
, extent(extent)
, side_length(2 * extent + 1)
, total_length(side_length * side_length * side_length)
, total_indexed(0)
, last_missing(0)
, stride(1, side_length, side_length * side_length)
, chunks(total_length, nullptr) {
	Scan();
}

ChunkIndex::~ChunkIndex() {
	Clear();
}

bool ChunkIndex::InRange(const Chunk::Pos &pos) const noexcept {
	return manhattan_radius(pos - base) <= extent;
}

int ChunkIndex::IndexOf(const Chunk::Pos &pos) const noexcept {
	Chunk::Pos mod_pos(
		GetCol(pos.x),
		GetCol(pos.y),
		GetCol(pos.z)
	);
	return mod_pos.x * stride.x
		+  mod_pos.y * stride.y
		+  mod_pos.z * stride.z;
}

Chunk::Pos ChunkIndex::PositionOf(int i) const noexcept {
	Chunk::Pos zero_pos(
		(i / stride.x) % side_length,
		(i / stride.y) % side_length,
		(i / stride.z) % side_length
	);
	Chunk::Pos zero_base(
		GetCol(base.x),
		GetCol(base.y),
		GetCol(base.z)
	);
	Chunk::Pos base_relative(zero_pos - zero_base);
	if (base_relative.x > extent) base_relative.x -= side_length;
	else if (base_relative.x < -extent) base_relative.x += side_length;
	if (base_relative.y > extent) base_relative.y -= side_length;
	else if (base_relative.y < -extent) base_relative.y += side_length;
	if (base_relative.z > extent) base_relative.z -= side_length;
	else if (base_relative.z < -extent) base_relative.z += side_length;
	return base + base_relative;
}

Chunk *ChunkIndex::Get(const Chunk::Pos &pos) noexcept {
	if (InRange(pos)) {
		return chunks[IndexOf(pos)];
	} else {
		return nullptr;
	}
}

const Chunk *ChunkIndex::Get(const Chunk::Pos &pos) const noexcept {
	if (InRange(pos)) {
		return chunks[IndexOf(pos)];
	} else {
		return nullptr;
	}
}

void ChunkIndex::Rebase(const Chunk::Pos &new_base) {
	if (new_base == base) return;

	Chunk::Pos diff(new_base - base);

	if (manhattan_radius(diff) > extent) {
		// that's more than half, so probably not worth shifting
		base = new_base;
		Clear();
		Scan();
		store.Clean();
		return;
	}

	while (diff.x > 0) {
		Shift(Block::FACE_RIGHT);
		--diff.x;
	}
	while (diff.x < 0) {
		Shift(Block::FACE_LEFT);
		++diff.x;
	}
	while (diff.y > 0) {
		Shift(Block::FACE_UP);
		--diff.y;
	}
	while (diff.y < 0) {
		Shift(Block::FACE_DOWN);
		++diff.y;
	}
	while (diff.z > 0) {
		Shift(Block::FACE_FRONT);
		--diff.z;
	}
	while (diff.z < 0) {
		Shift(Block::FACE_BACK);
		++diff.z;
	}
	store.Clean();
}

int ChunkIndex::GetCol(int c) const noexcept {
	c %= side_length;
	if (c < 0) c += side_length;
	return c;
}

void ChunkIndex::Shift(Block::Face f) {
	int a_axis = Block::Axis(f);
	int b_axis = (a_axis + 1) % 3;
	int c_axis = (a_axis + 2) % 3;
	int dir = Block::Direction(f);
	base[a_axis] += dir;
	int a = GetCol(base[a_axis] + (extent * dir));
	int a_stride = a * stride[a_axis];
	for (int b = 0; b < side_length; ++b) {
		int b_stride = b * stride[b_axis];
		for (int c = 0; c < side_length; ++c) {
			int bc_stride = b_stride + c * stride[c_axis];
			int index = a_stride + bc_stride;
			Unset(index);
			int neighbor = ((a - dir + side_length) % side_length) * stride[a_axis] + bc_stride;
			if (chunks[neighbor] && chunks[neighbor]->HasNeighbor(f)) {
				Set(index, chunks[neighbor]->GetNeighbor(f));
			}
		}
	}
}

void ChunkIndex::Clear() noexcept {
	for (int i = 0; i < total_length && total_indexed > 0; ++i) {
		Unset(i);
	}
}

void ChunkIndex::Scan() noexcept {
	for (Chunk &chunk : store) {
		Register(chunk);
	}
}

void ChunkIndex::Register(Chunk &chunk) noexcept {
	if (InRange(chunk.Position())) {
		Set(IndexOf(chunk.Position()), chunk);
	}
}

void ChunkIndex::Set(int index, Chunk &chunk) noexcept {
	Unset(index);
	chunks[index] = &chunk;
	chunk.Ref();
	++total_indexed;
}

void ChunkIndex::Unset(int index) noexcept {
	if (chunks[index]) {
		chunks[index]->UnRef();
		chunks[index] = nullptr;
		--total_indexed;
	}
}

Chunk::Pos ChunkIndex::NextMissing() noexcept {
	if (MissingChunks() > 0) {
		int roundtrip = last_missing;
		last_missing = (last_missing + 1) % total_length;
		while (chunks[last_missing]) {
			last_missing = (last_missing + 1) % total_length;
			if (last_missing == roundtrip) {
				break;
			}
		}
	}
	return PositionOf(last_missing);
}


ChunkStore::ChunkStore(const BlockTypeRegistry &types)
: types(types)
, loaded()
, free()
, indices() {

}

ChunkStore::~ChunkStore() {

}

ChunkIndex &ChunkStore::MakeIndex(const Chunk::Pos &pos, int extent) {
	indices.emplace_back(*this, pos, extent);
	return indices.back();
}

void ChunkStore::UnregisterIndex(ChunkIndex &index) {
	for (auto i = indices.begin(), end = indices.end(); i != end; ++i) {
		if (&*i == &index) {
			indices.erase(i);
			return;
		} else {
			++i;
		}
	}
}

Chunk *ChunkStore::Get(const Chunk::Pos &pos) {
	for (ChunkIndex &index : indices) {
		Chunk *chunk = index.Get(pos);
		if (chunk) {
			return chunk;
		}
	}
	return nullptr;
}

Chunk *ChunkStore::Allocate(const Chunk::Pos &pos) {
	Chunk *chunk = Get(pos);
	if (chunk) {
		return chunk;
	}
	if (free.empty()) {
		loaded.emplace(loaded.begin(), types);
	} else {
		loaded.splice(loaded.begin(), free, free.begin());
		loaded.front().Unlink();
	}
	chunk = &loaded.front();
	chunk->Position(pos);
	for (ChunkIndex &index : indices) {
		if (index.InRange(pos)) {
			index.Register(*chunk);
		}
	}
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		Block::Face face = Block::Face(i);
		Chunk::Pos neighbor_pos(pos + Block::FaceNormal(face));
		Chunk *neighbor = Get(neighbor_pos);
		if (neighbor) {
			chunk->SetNeighbor(face, *neighbor);
		}
	}
	return chunk;
}

bool ChunkStore::HasMissing() const noexcept {
	for (const ChunkIndex &index : indices) {
		if (index.MissingChunks() > 0) {
			return true;
		}
	}
	return false;
}

int ChunkStore::EstimateMissing() const noexcept {
	int missing = 0;
	for (const ChunkIndex &index : indices) {
		missing += index.MissingChunks();
	}
	return missing;
}

Chunk::Pos ChunkStore::NextMissing() noexcept {
	for (ChunkIndex &index : indices) {
		if (index.MissingChunks()) {
			return index.NextMissing();
		}
	}
	return Chunk::Pos(0, 0, 0);
}

void ChunkStore::Clean() {
	for (auto i = loaded.begin(), end = loaded.end(); i != end;) {
		if (i->Referenced()) {
			++i;
		} else {
			auto chunk = i;
			++i;
			free.splice(free.end(), loaded, chunk);
			chunk->Unlink();
			chunk->InvalidateModel();
		}
	}
}

}
