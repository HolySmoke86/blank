#include "GeometryTest.hpp"

#include "model/geometry.hpp"

#include <limits>
#include <glm/gtx/io.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::GeometryTest);


namespace blank {
namespace test {

void GeometryTest::setUp() {
}

void GeometryTest::tearDown() {
}


void GeometryTest::testRayAABBIntersection() {
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

}
}
