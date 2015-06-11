#include "GeometryTest.hpp"

#include "model/geometry.hpp"

#include <limits>

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
		Intersection(ray, box, M, &distance, &normal)
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		"intersection distance way off",
		0.0f, distance, delta
	);
}

}
}
