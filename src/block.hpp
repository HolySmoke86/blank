#ifndef BLANK_BLOCK_HPP_
#define BLANK_BLOCK_HPP_

#include "geometry.hpp"
#include "model.hpp"
#include "shape.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

/// single 1x1x1 cube
struct Block {

	using Type = unsigned short;
	using Pos = glm::vec3;

	enum Face {
		FACE_UP,
		FACE_DOWN,
		FACE_RIGHT,
		FACE_LEFT,
		FACE_FRONT,
		FACE_BACK,
		FACE_COUNT,
	};
	enum Turn {
		TURN_NONE,
		TURN_LEFT,
		TURN_AROUND,
		TURN_RIGHT,
		TURN_COUNT,
	};

	Type type;
	unsigned char orient;

	constexpr explicit Block(Type type = 0, Face face = FACE_UP, Turn turn = TURN_NONE)
	: type(type), orient(face * TURN_COUNT + turn) { }

	const glm::mat4 &Transform() const;

	Face GetFace() const { return Face(orient / 4); }
	void SetFace(Face face) { orient = face * TURN_COUNT + GetTurn(); }
	Turn GetTurn() const { return Turn(orient % 4); }
	void SetTurn(Turn turn) { orient = GetFace() * TURN_COUNT + turn; }

	static glm::tvec3<int> FaceNormal(Face face) {
		switch (face) {
			case FACE_UP:
				return { 0, 1, 0 };
			case FACE_DOWN:
				return { 0, -1, 0 };
			case FACE_RIGHT:
				return { 1, 0, 0 };
			case FACE_LEFT:
				return { -1, 0, 0 };
			case FACE_FRONT:
				return { 0, 0, 1 };
			case FACE_BACK:
				return { 0, 0, -1 };
			default:
				return { 0, 0, 0 };
		}
	}

};


/// attributes of a type of block
struct BlockType {

	const Shape *shape;
	glm::vec3 color;
	glm::vec3 outline_color;

	Block::Type id;

	int luminosity;

	bool visible;
	bool block_light;

	struct Faces {
		bool face[Block::FACE_COUNT];
		Faces &operator =(const Faces &other) {
			for (int i = 0; i < Block::FACE_COUNT; ++i) {
				face[i] = other.face[i];
			}
			return *this;
		}
		bool operator [](Block::Face f) const {
			return face[f];
		}
	} fill;

	explicit BlockType(
		bool v = false,
		const glm::vec3 &color = { 1, 1, 1 },
		const Shape *shape = &DEFAULT_SHAPE
	);

	static const NullShape DEFAULT_SHAPE;

	bool FaceFilled(const Block &, Block::Face) const;

	void FillModel(
		Model::Buffer &m,
		const glm::mat4 &transform = glm::mat4(1.0f),
		Model::Index idx_offset = 0
	) const;
	void FillOutlineModel(
		OutlineModel &m,
		const glm::vec3 &pos_offset = { 0, 0, 0 },
		OutlineModel::Index idx_offset = 0
	) const;

};


class BlockTypeRegistry {

public:
	BlockTypeRegistry();

public:
	Block::Type Add(const BlockType &);

	size_t Size() const { return types.size(); }

	BlockType *operator [](Block::Type id) { return &types[id]; }
	const BlockType *Get(Block::Type id) const { return &types[id]; }

private:
	std::vector<BlockType> types;

};

}

#endif
