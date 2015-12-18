#include "BlockLookup.hpp"
#include "Chunk.hpp"
#include "ChunkIndex.hpp"
#include "ChunkLoader.hpp"
#include "ChunkRenderer.hpp"
#include "ChunkStore.hpp"

#include "Generator.hpp"
#include "WorldCollision.hpp"
#include "../app/Assets.hpp"
#include "../geometry/distance.hpp"
#include "../graphics/BlockLighting.hpp"
#include "../graphics/BlockMesh.hpp"
#include "../graphics/Viewport.hpp"
#include "../io/WorldSave.hpp"

#include <algorithm>
#include <limits>
#include <ostream>
#include <queue>

#include <iostream>
#include <glm/gtx/io.hpp>


namespace blank {

constexpr int Chunk::side;
constexpr int Chunk::size;


Chunk::Chunk(const BlockTypeRegistry &types) noexcept
: types(&types)
, neighbor{0}
, gravity()
, blocks{}
, light{0}
, generated(false)
, lighted(false)
, position(0, 0, 0)
, ref_count(0)
, dirty_mesh(false)
, dirty_save(false) {

}

Chunk::Chunk(Chunk &&other) noexcept
: types(other.types)
, gravity(std::move(other.gravity))
, generated(other.generated)
, lighted(other.lighted)
, position(other.position)
, ref_count(other.ref_count)
, dirty_mesh(other.dirty_mesh)
, dirty_save(other.dirty_save) {
	std::copy(other.neighbor, other.neighbor + sizeof(neighbor), neighbor);
	std::copy(other.blocks, other.blocks + sizeof(blocks), blocks);
	std::copy(other.light, other.light + sizeof(light), light);
	other.ref_count = 0;
}

Chunk &Chunk::operator =(Chunk &&other) noexcept {
	types = other.types;
	std::copy(other.neighbor, other.neighbor + sizeof(neighbor), neighbor);
	gravity = std::move(other.gravity);
	std::copy(other.blocks, other.blocks + sizeof(blocks), blocks);
	std::copy(other.light, other.light + sizeof(light), light);
	generated = other.generated;
	lighted = other.lighted;
	position = other.position;
	std::swap(ref_count, other.ref_count);
	dirty_mesh = other.dirty_save;
	dirty_save = other.dirty_save;
	return *this;
}


namespace {

struct SetNode {

	Chunk *chunk;
	RoughLocation::Fine pos;

	SetNode(Chunk *chunk, RoughLocation::Fine pos)
	: chunk(chunk), pos(pos) { }

	int Get() const noexcept { return chunk->GetLight(pos); }
	void Set(int level) noexcept { chunk->SetLight(pos, level); }

	const BlockType &GetType() const noexcept { return chunk->Type(Chunk::ToIndex(pos)); }

	int EmitLevel() const noexcept { return GetType().luminosity; }
	bool EmitsLight() const noexcept { return EmitLevel() > 0; }

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

	UnsetNode(Chunk *chunk, RoughLocation::Fine pos)
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
				if (other.Get() != 0 && other.Get() < node.level) {
					if (other.EmitsLight()) {
						other.Set(other.EmitLevel());
						light_queue.emplace(other);
					} else {
						other.Set(0);
					}
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

	if (old_type.gravity && !new_type.gravity) {
		gravity.erase(index);
	} else if (new_type.gravity && !old_type.gravity) {
		gravity.insert(index);
	}

	if (!lighted || &old_type == &new_type) return;

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
		RoughLocation::Fine pos(ToPos(index));
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

void Chunk::ScanLights() {
	int idx = 0;
	RoughLocation::Fine pos(0, 0, 0);
	for (; pos.z < side; ++pos.z) {
		for (pos.y = 0; pos.y < side; ++pos.y) {
			for (pos.x = 0; pos.x < side; ++pos.x, ++idx) {
				const BlockType &type = Type(blocks[idx]);
				if (type.luminosity) {
					SetLight(idx, type.luminosity);
					light_queue.emplace(this, pos);
				}
			}
		}
	}
	work_light();
	lighted = true;
}

void Chunk::ScanActive() {
	gravity.clear();
	for (int index = 0; index < size; ++index) {
		if (Type(index).gravity) {
			gravity.insert(gravity.end(), index);
		}
	}
}

void Chunk::SetNeighbor(Block::Face face, Chunk &other) noexcept {
	neighbor[face] = &other;
	other.neighbor[Block::Opposite(face)] = this;
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

float Chunk::GetVertexLight(const RoughLocation::Fine &pos, const BlockMesh::Position &vtx, const EntityMesh::Normal &norm) const noexcept {
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


glm::vec3 Chunk::GravityAt(const ExactLocation &coords) const noexcept {
	glm::vec3 grav(0.0f);
	for (int index : gravity) {
		RoughLocation::Fine block_pos(ToPos(index));
		ExactLocation block_coords(position, ToCoords(block_pos));
		// trust that block type hasn't changed
		grav += Type(index).gravity->GetGravity(
			coords.Difference(block_coords).Absolute(),
			ToTransform(block_pos, index));
	}
	return grav;
}


bool Chunk::IsSurface(const RoughLocation::Fine &pos) const noexcept {
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
	const ExactLocation::Coarse &reference,
	WorldCollision &coll
) noexcept {
	int idx = 0;
	coll.chunk = this;
	coll.block = -1;
	coll.depth = std::numeric_limits<float>::infinity();
	for (int z = 0; z < side; ++z) {
		for (int y = 0; y < side; ++y) {
			for (int x = 0; x < side; ++x, ++idx) {
				const BlockType &type = Type(idx);
				if (!type.collision || !type.shape) {
					continue;
				}
				float cur_dist;
				glm::vec3 cur_norm;
				if (type.shape->Intersects(ray, ToTransform(reference, RoughLocation::Fine(x, y, z), idx), cur_dist, cur_norm)) {
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

	// box's origin relative to the chunk
	const glm::vec3 box_coords(Mbox[3] - Mchunk[3]);
	const float box_rad = box.OriginRadius();

	// assume a bounding radius of 2 for blocks
	constexpr float block_rad = 2.0f;
	const float bb_radius = box_rad + block_rad;

	const RoughLocation::Fine begin(max(
		RoughLocation::Fine(0),
		RoughLocation::Fine(floor(box_coords - bb_radius))
	));
	const RoughLocation::Fine end(min(
		RoughLocation::Fine(side - 1),
		RoughLocation::Fine(ceil(box_coords + bb_radius))
	) - 1);

	for (RoughLocation::Fine pos(begin); pos.z < end.y; ++pos.z) {
		for (pos.y = begin.y; pos.y < end.y; ++pos.y) {
			for (pos.x = begin.x; pos.x < end.x; ++pos.x) {
				int idx = ToIndex(pos);
				const BlockType &type = Type(idx);
				if (!type.collision || !type.shape) {
					continue;
				}
				if (type.shape->Intersects(Mchunk * ToTransform(pos, idx), box, Mbox, penetration, normal)) {
					col.emplace_back(this, idx, penetration, normal);
					any = true;
				}
			}
		}
	}
	return any;
}

bool Chunk::Intersection(
	const Entity &entity,
	const glm::mat4 &Mentity,
	const glm::mat4 &Mchunk,
	std::vector<WorldCollision> &col
) noexcept {
	// entity's origin relative to the chunk
	const glm::vec3 entity_coords(Mentity[3] - Mchunk[3]);
	const float ec_radius = entity.Radius() + Radius();

	if (distance2(entity_coords, Center()) > ec_radius * ec_radius) {
		return false;
	}

	bool any = false;
	float penetration;
	glm::vec3 normal;

	// assume a bounding radius of 2 for blocks
	constexpr float block_rad = 2.0f;
	const float eb_radius = entity.Radius() + block_rad;

	const RoughLocation::Fine begin(max(
		RoughLocation::Fine(0),
		RoughLocation::Fine(floor(entity_coords - eb_radius))
	));
	const RoughLocation::Fine end(min(
		RoughLocation::Fine(side),
		RoughLocation::Fine(ceil(entity_coords + eb_radius))
	));

	for (RoughLocation::Fine pos(begin); pos.z < end.z; ++pos.z) {
		for (pos.y = begin.y; pos.y < end.y; ++pos.y) {
			for (pos.x = begin.x; pos.x < end.x; ++pos.x) {
				int idx = ToIndex(pos);
				const BlockType &type = Type(idx);
				if (!type.collision || !type.shape) {
					continue;
				}
				if (type.shape->Intersects(Mchunk * ToTransform(pos, idx), entity.Bounds(), Mentity, penetration, normal)) {
					col.emplace_back(this, idx, penetration, normal);
					any = true;
				}
			}
		}
	}
	return any;
}


namespace {

BlockMesh::Buffer buf;

}

void Chunk::Update(BlockMesh &model) noexcept {
	int vtx_count = 0, idx_count = 0;
	for (const auto &block : blocks) {
		const BlockType &type = Type(block);
		if (type.visible && type.shape) {
			vtx_count += type.shape->VertexCount();
			idx_count += type.shape->IndexCount();
		}
	}
	buf.Clear();
	buf.Reserve(vtx_count, idx_count);

	if (idx_count > 0) {
		int idx = 0;
		BlockMesh::Index vtx_counter = 0;
		for (size_t z = 0; z < side; ++z) {
			for (size_t y = 0; y < side; ++y) {
				for (size_t x = 0; x < side; ++x, ++idx) {
					const BlockType &type = Type(BlockAt(idx));
					const RoughLocation::Fine pos(x, y, z);

					if (!type.visible || !type.shape || Obstructed(pos).All()) continue;

					type.FillBlockMesh(buf, ToTransform(pos, idx), vtx_counter);
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
	}

	model.Update(buf);
	ClearMesh();
}

Block::FaceSet Chunk::Obstructed(const RoughLocation::Fine &pos) const noexcept {
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

glm::mat4 Chunk::ToTransform(const RoughLocation::Fine &pos, int idx) const noexcept {
	return glm::translate(ToCoords(pos)) * BlockAt(idx).Transform();
}

glm::mat4 Chunk::ToTransform(const ExactLocation::Coarse &ref, const RoughLocation::Fine &pos, int idx) const noexcept {
	return glm::translate(ExactLocation::Fine((position - ref) * ExactLocation::Extent()) + ToCoords(pos)) * BlockAt(idx).Transform();
}


BlockLookup::BlockLookup(Chunk *c, const RoughLocation::Fine &p) noexcept
: chunk(c), pos(p) {
	while (pos.x >= Chunk::side) {
		if (chunk->HasNeighbor(Block::FACE_RIGHT)) {
			chunk = &chunk->GetNeighbor(Block::FACE_RIGHT);
			pos.x -= Chunk::side;
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.x < 0) {
		if (chunk->HasNeighbor(Block::FACE_LEFT)) {
			chunk = &chunk->GetNeighbor(Block::FACE_LEFT);
			pos.x += Chunk::side;
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.y >= Chunk::side) {
		if (chunk->HasNeighbor(Block::FACE_UP)) {
			chunk = &chunk->GetNeighbor(Block::FACE_UP);
			pos.y -= Chunk::side;
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.y < 0) {
		if (chunk->HasNeighbor(Block::FACE_DOWN)) {
			chunk = &chunk->GetNeighbor(Block::FACE_DOWN);
			pos.y += Chunk::side;
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.z >= Chunk::side) {
		if (chunk->HasNeighbor(Block::FACE_FRONT)) {
			chunk = &chunk->GetNeighbor(Block::FACE_FRONT);
			pos.z -= Chunk::side;
		} else {
			chunk = nullptr;
			return;
		}
	}
	while (pos.z < 0) {
		if (chunk->HasNeighbor(Block::FACE_BACK)) {
			chunk = &chunk->GetNeighbor(Block::FACE_BACK);
			pos.z += Chunk::side;
		} else {
			chunk = nullptr;
			return;
		}
	}
}

BlockLookup::BlockLookup(Chunk *c, const RoughLocation::Fine &p, Block::Face face) noexcept
: chunk(c), pos(p) {
	pos += Block::FaceNormal(face);
	if (!Chunk::InBounds(pos)) {
		pos -= Block::FaceNormal(face) * ExactLocation::Extent();
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

	ExactLocation::Coarse pos = store.NextMissing();
	Chunk *chunk = store.Allocate(pos);
	if (!chunk) {
		// chunk store corrupted?
		return false;
	}

	bool generated = false;
	if (save.Exists(pos)) {
		save.Read(*chunk);
	} else {
		gen(*chunk);
		generated = true;
	}

	ChunkIndex *index = store.ClosestIndex(pos);
	if (!index) {
		return generated;
	}

	ExactLocation::Coarse begin(pos - ExactLocation::Coarse(1));
	ExactLocation::Coarse end(pos + ExactLocation::Coarse(2));
	for (ExactLocation::Coarse iter(begin); iter.z < end.z; ++iter.z) {
		for (iter.y = begin.y; iter.y < end.y; ++iter.y) {
			for (iter.x = begin.x; iter.x < end.x; ++iter.x) {
				if (index->IsBorder(iter)) continue;
				Chunk *light_chunk = index->Get(iter);
				if (!light_chunk) continue;
				if (index->HasAllSurrounding(iter)) {
					if (!light_chunk->Lighted()) {
						light_chunk->ScanLights();
					} else {
						light_chunk->InvalidateMesh();
					}
				}
			}
		}
	}

	return generated;
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

void ChunkRenderer::LoadTextures(const AssetLoader &loader, const ResourceIndex &tex_index) {
	block_tex.Bind();
	loader.LoadTextures(tex_index, block_tex);
	block_tex.FilterNearest();
}

void ChunkRenderer::Update(int dt) {
	for (int i = 0, updates = 0; updates < dt && i < index.TotalChunks(); ++i) {
		if (!index[i]) continue;
		if (!index[i]->Lighted() && index.HasAllSurrounding(index[i]->Position())) {
			index[i]->ScanLights();
		}
		if (index[i]->ShouldUpdateMesh()) {
			index[i]->Update(models[i]);
			++updates;
		}
	}
}

void ChunkRenderer::Render(Viewport &viewport) {
	BlockLighting &chunk_prog = viewport.ChunkProgram();
	chunk_prog.SetTexture(block_tex);
	chunk_prog.SetFogDensity(fog_density);

	Frustum frustum(transpose(chunk_prog.GetVP()));
	AABB box;

	for (int i = 0; i < index.TotalChunks(); ++i) {
		if (!index[i]) continue;
		box.min = (index[i]->Position() - index.Base()) * ExactLocation::Extent();
		box.max = box.min + ExactLocation::FExtent();

		if (!CullTest(box, frustum)) {
			if (index[i]->ShouldUpdateMesh()) {
				index[i]->Update(models[i]);
			}
			if (!models[i].Empty()) {
				chunk_prog.SetM(index[i]->Transform(index.Base()));
				models[i].Draw();
			}
		}
	}
}


ChunkIndex::ChunkIndex(ChunkStore &store, const ExactLocation::Coarse &base, int extent)
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

bool ChunkIndex::InRange(const ExactLocation::Coarse &pos) const noexcept {
	return Distance(pos) <= extent;
}

bool ChunkIndex::IsBorder(const ExactLocation::Coarse &pos) const noexcept {
	return Distance(pos) == extent;
}

int ChunkIndex::Distance(const ExactLocation::Coarse &pos) const noexcept {
	return manhattan_radius(pos - base);
}

bool ChunkIndex::HasAllSurrounding(const ExactLocation::Coarse &pos) const noexcept {
	ExactLocation::Coarse begin(pos - ExactLocation::Coarse(1));
	ExactLocation::Coarse end(pos + ExactLocation::Coarse(2));
	for (ExactLocation::Coarse iter(begin); iter.z < end.z; ++iter.z) {
		for (iter.y = begin.y; iter.y < end.y; ++iter.y) {
			for (iter.x = begin.x; iter.x < end.x; ++iter.x) {
				if (!Get(iter)) return false;
			}
		}
	}
	return true;
}

int ChunkIndex::IndexOf(const ExactLocation::Coarse &pos) const noexcept {
	ExactLocation::Coarse mod_pos(
		GetCol(pos.x),
		GetCol(pos.y),
		GetCol(pos.z)
	);
	return mod_pos.x * stride.x
		+  mod_pos.y * stride.y
		+  mod_pos.z * stride.z;
}

ExactLocation::Coarse ChunkIndex::PositionOf(int i) const noexcept {
	ExactLocation::Coarse zero_pos(
		(i / stride.x) % side_length,
		(i / stride.y) % side_length,
		(i / stride.z) % side_length
	);
	ExactLocation::Coarse zero_base(
		GetCol(base.x),
		GetCol(base.y),
		GetCol(base.z)
	);
	ExactLocation::Coarse base_relative(zero_pos - zero_base);
	if (base_relative.x > extent) base_relative.x -= side_length;
	else if (base_relative.x < -extent) base_relative.x += side_length;
	if (base_relative.y > extent) base_relative.y -= side_length;
	else if (base_relative.y < -extent) base_relative.y += side_length;
	if (base_relative.z > extent) base_relative.z -= side_length;
	else if (base_relative.z < -extent) base_relative.z += side_length;
	return base + base_relative;
}

Chunk *ChunkIndex::Get(const ExactLocation::Coarse &pos) noexcept {
	if (InRange(pos)) {
		return chunks[IndexOf(pos)];
	} else {
		return nullptr;
	}
}

const Chunk *ChunkIndex::Get(const ExactLocation::Coarse &pos) const noexcept {
	if (InRange(pos)) {
		return chunks[IndexOf(pos)];
	} else {
		return nullptr;
	}
}

void ChunkIndex::Rebase(const ExactLocation::Coarse &new_base) {
	if (new_base == base) return;

	ExactLocation::Coarse diff(new_base - base);

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

ExactLocation::Coarse ChunkIndex::NextMissing() noexcept {
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

ChunkIndex &ChunkStore::MakeIndex(const ExactLocation::Coarse &pos, int extent) {
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

ChunkIndex *ChunkStore::ClosestIndex(const ExactLocation::Coarse &pos) {
	ChunkIndex *closest_index = nullptr;
	int closest_distance = std::numeric_limits<int>::max();

	for (ChunkIndex &index : indices) {
		int distance = index.Distance(pos);
		if (distance < closest_distance) {
			closest_index = &index;
			closest_distance = distance;
		}
	}

	return closest_index;
}

Chunk *ChunkStore::Get(const ExactLocation::Coarse &pos) noexcept {
	for (ChunkIndex &index : indices) {
		Chunk *chunk = index.Get(pos);
		if (chunk) {
			return chunk;
		}
	}
	return nullptr;
}

const Chunk *ChunkStore::Get(const ExactLocation::Coarse &pos) const noexcept {
	for (const ChunkIndex &index : indices) {
		const Chunk *chunk = index.Get(pos);
		if (chunk) {
			return chunk;
		}
	}
	return nullptr;
}

Chunk *ChunkStore::Allocate(const ExactLocation::Coarse &pos) {
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
		ExactLocation::Coarse neighbor_pos(pos + Block::FaceNormal(face));
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

ExactLocation::Coarse ChunkStore::NextMissing() noexcept {
	for (ChunkIndex &index : indices) {
		if (index.MissingChunks()) {
			return index.NextMissing();
		}
	}
	return ExactLocation::Coarse(0, 0, 0);
}

void ChunkStore::Clean() {
	for (auto i = loaded.begin(), end = loaded.end(); i != end;) {
		if (i->Referenced() || i->ShouldUpdateSave()) {
			++i;
		} else {
			auto chunk = i;
			++i;
			free.splice(free.end(), loaded, chunk);
			chunk->Unlink();
			chunk->InvalidateMesh();
		}
	}
}

}
