#ifndef BLANK_WORLD_BLOCK_HPP_
#define BLANK_WORLD_BLOCK_HPP_

#include <iosfwd>
#include <glm/glm.hpp>


namespace blank {

/// single 1x1x1 cube
struct Block {

	using Type = unsigned short;

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

	constexpr explicit Block(Type type = 0, Face face = FACE_UP, Turn turn = TURN_NONE) noexcept
	: type(type), orient(face * TURN_COUNT + turn) { }

	const glm::mat4 &Transform() const noexcept { return orient2transform[orient]; }

	Face GetFace() const noexcept { return Face(orient / TURN_COUNT); }
	void SetFace(Face face) noexcept { orient = face * TURN_COUNT + GetTurn(); }
	Turn GetTurn() const noexcept { return Turn(orient % TURN_COUNT); }
	void SetTurn(Turn turn) noexcept { orient = GetFace() * TURN_COUNT + turn; }

	Face OrientedFace(Face f) const noexcept { return orient2face[orient][f]; }

	static Face Opposite(Face f) noexcept {
		return Face(f ^ 1);
	}

	static int Axis(Face f) noexcept {
		switch (f) {
			case FACE_UP:
			case FACE_DOWN:
				return 1;
			default:
			case FACE_RIGHT:
			case FACE_LEFT:
				return 0;
			case FACE_FRONT:
			case FACE_BACK:
				return 2;
		}
	}

	/// returns 1 for pro-axis, -1 for retro-axis, 0 for invalid faces
	static int Direction(Face f) noexcept {
		switch (f) {
			case FACE_RIGHT:
			case FACE_UP:
			case FACE_FRONT:
				return 1;
			case FACE_LEFT:
			case FACE_DOWN:
			case FACE_BACK:
				return -1;
			default:
				return 0;
		}
	}

	static glm::ivec3 FaceNormal(Face face) noexcept {
		return face2normal[face];
	}

	static Face NormalFace(const glm::vec3 &norm) noexcept {
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
			value &= ~Mask(f);
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
	static const glm::ivec3 face2normal[6];
	static const glm::mat4 orient2transform[ORIENT_COUNT];
	static const Face orient2face[ORIENT_COUNT][FACE_COUNT];

};

inline bool operator ==(const Block &a, const Block &b) {
	return a.type == b.type && a.orient == b.orient;
}

inline bool operator !=(const Block &a, const Block &b) {
	return !(a == b);
}

std::ostream &operator <<(std::ostream &, const Block &);
std::ostream &operator <<(std::ostream &, const Block::Face &);
std::ostream &operator <<(std::ostream &, const Block::Turn &);

}

#endif
