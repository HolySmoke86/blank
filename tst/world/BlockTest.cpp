#include "BlockTest.hpp"

#include "world/Block.hpp"

#include <glm/gtx/io.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::BlockTest);


namespace blank {
namespace test {

void BlockTest::setUp() {
}

void BlockTest::tearDown() {
}


void BlockTest::testFaceOpposite() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"DOWN not opposite of UP",
		Block::FACE_DOWN, Block::Opposite(Block::FACE_UP)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"UP not opposite of DOWN",
		Block::FACE_UP, Block::Opposite(Block::FACE_DOWN)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"LEFT not opposite of RIGHT",
		Block::FACE_LEFT, Block::Opposite(Block::FACE_RIGHT)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"RIGHT not opposite of LEFT",
		Block::FACE_RIGHT, Block::Opposite(Block::FACE_LEFT)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"BACK not opposite of FRONT",
		Block::FACE_BACK, Block::Opposite(Block::FACE_FRONT)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"FRONT not opposite of BACk",
		Block::FACE_FRONT, Block::Opposite(Block::FACE_BACK)
	);
}

void BlockTest::testFaceAxis() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"1/Y not axis of UP",
		1, Block::Axis(Block::FACE_UP)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"1/Y not axis of DOWN",
		1, Block::Axis(Block::FACE_DOWN)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"0/X not axis of RIGHT",
		0, Block::Axis(Block::FACE_RIGHT)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"0/X not axis of LEFT",
		0, Block::Axis(Block::FACE_LEFT)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"2/Z not axis of FRONT",
		2, Block::Axis(Block::FACE_FRONT)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"2/Z not axis of BACK",
		2, Block::Axis(Block::FACE_BACK)
	);
}

void BlockTest::testFaceNormal() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"[ 0, 1, 0 ] not normal of UP",
		glm::tvec3<int>(0, 1, 0), Block::FaceNormal(Block::FACE_UP)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"[ 0, -1, 0 ] not normal of DOWN",
		glm::tvec3<int>(0, -1, 0), Block::FaceNormal(Block::FACE_DOWN)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"[ 1, 0, 0 ] not normal of RIGHT",
		glm::tvec3<int>(1, 0, 0), Block::FaceNormal(Block::FACE_RIGHT)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"[ -1, 0, 0 ] not normal of LEFT",
		glm::tvec3<int>(-1, 0, 0), Block::FaceNormal(Block::FACE_LEFT)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"[ 0, 0, 1 ] not normal of FRONT",
		glm::tvec3<int>(0, 0, 1), Block::FaceNormal(Block::FACE_FRONT)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"[ 0, 0, -1 ] not normal of BACK",
		glm::tvec3<int>(0, 0, -1), Block::FaceNormal(Block::FACE_BACK)
	);
}

void BlockTest::testNormalFace() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"UP not face of [ 0, 1, 0 ]",
		Block::FACE_UP, Block::NormalFace(glm::vec3(0, 1, 0))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"DOWN not face of [ 0, -1, 0 ]",
		Block::FACE_DOWN, Block::NormalFace(glm::vec3(0, -1, 0))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"RIGHT not face of [ 1, 0, 0 ]",
		Block::FACE_RIGHT, Block::NormalFace(glm::vec3(1, 0, 0))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"LEFT not face of [ -1, 0, 0 ]",
		Block::FACE_LEFT, Block::NormalFace(glm::vec3(-1, 0, 0))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"FRONT not face of [ 0, 0, 1 ]",
		Block::FACE_FRONT, Block::NormalFace(glm::vec3(0, 0, 1))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"BACK not face of [ 0, 0, -1 ]",
		Block::FACE_BACK, Block::NormalFace(glm::vec3(0, 0, -1))
	);
}


void BlockTest::testFaceSet() {
	Block::FaceSet set;
	CPPUNIT_ASSERT_MESSAGE(
		"default faceset not empty",
		set.Empty()
	);
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		CPPUNIT_ASSERT_MESSAGE(
			"something set on default faceset",
			!set.IsSet(Block::Face(i))
		);
	}
	set.Fill();
	CPPUNIT_ASSERT_MESSAGE(
		"not all set on filled faceset",
		set.All()
	);
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		CPPUNIT_ASSERT_MESSAGE(
			"something not set on filled faceset",
			set.IsSet(Block::Face(i))
		);
	}
	set.Clear();
	CPPUNIT_ASSERT_MESSAGE(
		"cleared faceset not empty",
		set.Empty()
	);
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		CPPUNIT_ASSERT_MESSAGE(
			"something set on cleared faceset",
			!set.IsSet(Block::Face(i))
		);
		set.Set(Block::Face(i));
		CPPUNIT_ASSERT_MESSAGE(
			"face not set after setting",
			set.IsSet(Block::Face(i))
		);
	}
	CPPUNIT_ASSERT_MESSAGE(
		"set not filled after setting all faces",
		set.All()
	);
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		CPPUNIT_ASSERT_MESSAGE(
			"something not set after setting all faces",
			set.IsSet(Block::Face(i))
		);
		set.Unset(Block::Face(i));
		CPPUNIT_ASSERT_MESSAGE(
			"face set after unsetting",
			!set.IsSet(Block::Face(i))
		);
	}
	CPPUNIT_ASSERT_MESSAGE(
		"set not empty after unsetting all faces",
		set.Empty()
	);
}


void BlockTest::testDefaultBlock() {
	Block block;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"0 type id of default block",
		Block::Type(0), block.type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"identity transform of default block",
		glm::mat4(1), block.Transform()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"no turn of default block",
		Block::TURN_NONE, block.GetTurn()
	);
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"face is oriented face of default block",
			Block::Face(i), block.OrientedFace(Block::Face(i))
		);
	}
}

}
}
