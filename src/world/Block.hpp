#ifndef BLANK_WORLD_BLOCK_HPP_
#define BLANK_WORLD_BLOCK_HPP_

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

	static glm::tvec3<int> FaceNormal(Face face) noexcept {
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

}

#endif
