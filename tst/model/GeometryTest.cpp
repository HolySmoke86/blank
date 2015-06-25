#include "GeometryTest.hpp"

#include "model/geometry.hpp"

#include <limits>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::GeometryTest);


namespace blank {
namespace test {

void GeometryTest::setUp() {
}

void GeometryTest::tearDown() {
}


void GeometryTest::testRayBoxIntersection() {
	Ray ray{ { 0, 0, 0 }, { 1, 0, 0 } }; // at origin, pointing right
	AABB box{ { -1, -1, -1 }, { 1, 1, 1 } }; // 2x2x2 cube centered around origin
	glm::mat4 M(1); // no transformation

	const float delta = std::numeric_limits<float>::epsilon();

	float distance = 0;
	glm::vec3 normal(0);

	CPPUNIT_ASSERT_MESSAGE(
		"ray at origin not intersecting box at origin",
		Intersection(ray, box, M, &distance)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"intersection distance way off",
		0.0f, distance, delta
	);
	// normal undefined, so can't test

	// move ray outside the box, but have it still point at it
	// should be 4 units to the left now
	ray.orig.x = -5;
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing at box doesn't intersect",
		Intersection(ray, box, M, &distance, &normal)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"intersection distance way off",
		4.0f, distance, delta
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong surface normal at intersection point",
		glm::vec3(-1, 0, 0), normal
	);

	// move ray to the other side, so it's pointing away now
	ray.orig.x = 5;
	CPPUNIT_ASSERT_MESSAGE(
		"ray pointing away from box still intersects",
		!Intersection(ray, box, M)
	);
}

void GeometryTest::testBoxBoxIntersection() {
	const float delta = std::numeric_limits<float>::epsilon();
	float depth = 0;
	glm::vec3 normal(0);

	AABB box{ { -1, -1, -1 }, { 1, 1, 1 } }; // 2x2x2 cube centered around origin
	glm::mat4 Ma(1); // identity
	glm::mat4 Mb(1); // identity
	// they're identical, so should probably intersect ^^

	CPPUNIT_ASSERT_MESSAGE(
		"identical OBBs don't intersect",
		Intersection(box, Ma, box, Mb, depth, normal)
	);
	// depth is two, but normal can be any
	// (will probably be the first axis of box a, but any is valid)
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"penetration depth of coincidental 2x2x2 boxes is not 2",
		2.0f, depth, delta
	);

	Ma = glm::translate(glm::vec3(-2, 0, 0)); // 2 to the left
	Mb = glm::translate(glm::vec3(2, 0, 0)); // 2 to the right
	CPPUNIT_ASSERT_MESSAGE(
		"distant OBBs intersect (2 apart, no rotation)",
		!Intersection(box, Ma, box, Mb, depth, normal)
	);
	// depth and normal undefined for non-intersecting objects

	Ma = glm::rotate(PI_0p25, glm::vec3(0, 0, 1)); // rotated 45° around Z
	Mb = glm::translate(glm::vec3(2.4, 0, 0)); // 2.4 to the right
	// they should barely touch. intersect by about sqrt(2) - 1.4 if my head works
	CPPUNIT_ASSERT_MESSAGE(
		"OBBs don't intersect (one rotated by 45°)",
		Intersection(box, Ma, box, Mb, depth, normal)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"bad penetration depth (with rotation)",
		0.01421356237309504880f, depth, delta
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad intersection normal (with rotation)",
		glm::vec3(1, 0, 0), abs(normal) // normal can be in + or - x, therefore abs()
	);

	Mb = glm::translate(glm::vec3(3, 0, 0)); // 3 to the right
	CPPUNIT_ASSERT_MESSAGE(
		"OBBs intersect (one rotated by 45°)",
		!Intersection(box, Ma, box, Mb, depth, normal)
	);
}

}
}
