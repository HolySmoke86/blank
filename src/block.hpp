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

	static constexpr int ORIENT_COUNT = FACE_COUNT * TURN_COUNT;

	Type type;
	unsigned char orient;

	constexpr explicit Block(Type type = 0, Face face = FACE_UP, Turn turn = TURN_NONE)
	: type(type), orient(face * TURN_COUNT + turn) { }

	const glm::mat4 &Transform() const { return orient2transform[orient]; }

	Face GetFace() const { return Face(orient / TURN_COUNT); }
	void SetFace(Face face) { orient = face * TURN_COUNT + GetTurn(); }
	Turn GetTurn() const { return Turn(orient % TURN_COUNT); }
	void SetTurn(Turn turn) { orient = GetFace() * TURN_COUNT + turn; }

	Face OrientedFace(Face f) const { return orient2face[orient][f]; }

	static Face Opposite(Face f) {
		return Face(f ^ 1);
	}

	static glm::tvec3<int> FaceNormal(Face face) {
		return face2normal[face];
	}

	static Face NormalFace(const glm::vec3 &norm) {
		const glm::vec3 anorm(abs(norm));
		if (anorm.x > anorm.y) {
			if (anorm.x > anorm.z) {
				return norm.x > 0.0f ? FACE_RIGHT : FACE_LEFT;
			} else {
				return norm.z > 0.0f ? FACE_FRONT : FACE_BACK;
			}
		} else {
			if (anorm.y > anorm.z) {
				return norm.y > 0.0f ? FACE_UP : FACE_DOWN;
			} else {
				return norm.z > 0.0f ? FACE_FRONT : FACE_BACK;
			}
		}
	}

	struct FaceSet {

		explicit FaceSet(unsigned char v = 0)
		: value(v) { }

		bool IsSet(Face f) const {
			return value & Mask(f);
		}
		void Set(Face f) {
			value |= Mask(f);
		}
		void Unset(Face f) {
			value |= ~Mask(f);
		}

		void Clear() {
			value = 0;
		}
		void Fill() {
			value = Mask(FACE_COUNT) - 1;
		}

		bool Empty() const {
			return value == 0;
		}
		bool All() const {
			return value == Mask(FACE_COUNT) - 1;
		}

		unsigned char Mask(Face f) const {
			return 1 << f;
		}

		unsigned char value;

	};

private:
	static const glm::tvec3<int> face2normal[6];
	static const glm::mat4 orient2transform[ORIENT_COUNT];
	static const Face orient2face[ORIENT_COUNT][FACE_COUNT];

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

	bool FaceFilled(const Block &block, Block::Face face) const {
		return fill[block.OrientedFace(face)];
	}

	void FillModel(
		Model::Buffer &m,
		const glm::mat4 &transform = glm::mat4(1.0f),
		Model::Index idx_offset = 0
	) const;
	void FillBlockModel(
		BlockModel::Buffer &m,
		const glm::mat4 &transform = glm::mat4(1.0f),
		BlockModel::Index idx_offset = 0
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
