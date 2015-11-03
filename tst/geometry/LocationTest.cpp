#include "LocationTest.hpp"

#include <limits>
#include <glm/gtx/io.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::LocationTest);


namespace blank {
namespace test {

void LocationTest::setUp() {
}

void LocationTest::tearDown() {
}


void LocationTest::testSanitize() {
	{
		RoughLocation loc({ 0, 0, 0 }, { 0, 0, 0 });
		loc.Sanitize();
		AssertEqual(
			"sanitize rough zero location",
			RoughLocation({ 0, 0, 0 }, { 0, 0, 0 }), loc
		);
	}
	{
		ExactLocation loc({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f });
		loc.Sanitize();
		AssertEqual(
			"sanitize exact zero location",
			ExactLocation({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f }), loc
		);
	}
	{
		ExactLocation loc({ 0, 0, 0 }, { 15.9f, 0.0f, 0.0f });
		loc.Sanitize();
		AssertEqual(
			"sanitize exact location near upper boundary",
			ExactLocation({ 0, 0, 0 }, { 15.9f, 0.0f, 0.0f }), loc
		);
	}
	{
		RoughLocation loc({ 0, 0, 0 }, { 0, 16, 0 });
		loc.Sanitize();
		AssertEqual(
			"sanitize rough location",
			RoughLocation({ 0, 1, 0 }, { 0, 0, 0 }), loc
		);
	}
	{
		ExactLocation loc({ 0, 0, 0 }, { 0.0f, 16.0f, 0.0f });
		loc.Sanitize();
		AssertEqual(
			"sanitize exact location",
			ExactLocation({ 0, 1, 0 }, { 0.0f, 0.0f, 0.0f }), loc
		);
	}
	{
		RoughLocation loc({ 0, 0, 0 }, { 0, 0, -1 });
		loc.Sanitize();
		AssertEqual(
			"sanitize rough negative location",
			RoughLocation({ 0, 0, -1 }, { 0, 0, 15 }), loc
		);
	}
	{
		ExactLocation loc({ 0, 0, 0 }, { 0.0f, 0.0f, -1.0f });
		loc.Sanitize();
		AssertEqual(
			"sanitize exact negative location",
			ExactLocation({ 0, 0, -1 }, { 0.0f, 0.0f, 15.0f }), loc
		);
	}
	{
		RoughLocation loc({ 0, 0, 0 }, { 0, 41585, 0 });
		loc.Sanitize();
		AssertEqual(
			"sanitize rough really far location",
			RoughLocation({ 0, 2599, 0 }, { 0, 1, 0 }), loc
		);
	}
	{
		ExactLocation loc({ 0, 0, 0 }, { 0.0f, 41585.0f, 0.0f });
		loc.Sanitize();
		AssertEqual(
			"sanitize exact really far location",
			ExactLocation({ 0, 2599, 0 }, { 0.0f, 1.0f, 0.0f }), loc
		);
	}
	{
		RoughLocation loc({ 0, 0, 0 }, { -208005, 0, 0 });
		loc.Sanitize();
		AssertEqual(
			"sanitize rough really far negative location",
			RoughLocation({ -13001, 0, 0 }, { 11, 0, 0 }), loc
		);
	}
	{
		ExactLocation loc({ 0, 0, 0 }, { -208005.0f, 0.0f, 0.0f });
		loc.Sanitize();
		AssertEqual(
			"sanitize exact really far negative location",
			ExactLocation({ -13001, 0, 0 }, { 11.0f, 0.0f, 0.0f }), loc
		);
	}
	{
		RoughLocation loc({ 0, -2, 0 }, { 0, 16, 0 });
		loc.Sanitize();
		AssertEqual(
			"sanitize rough location with non-zero chunk",
			RoughLocation({ 0, -1, 0 }, { 0, 0, 0 }), loc
		);
	}
	{
		ExactLocation loc({ 0, -2, 0 }, { 0.0f, 16.0f, 0.0f });
		loc.Sanitize();
		AssertEqual(
			"sanitize exact location with non-zero chunk",
			ExactLocation({ 0, -1, 0 }, { 0.0f, 0.0f, 0.0f }), loc
		);
	}
	{
		RoughLocation loc({ 0, 0, 5 }, { 0, 0, -33 });
		loc.Sanitize();
		AssertEqual(
			"sanitize rough negative location with non-zero chunk",
			RoughLocation({ 0, 0, 2 }, { 0, 0, 15 }), loc
		);
	}
	{
		ExactLocation loc({ 0, 0, 5 }, { 0.0f, 0.0f, -33.0f });
		loc.Sanitize();
		AssertEqual(
			"sanitize exact negative location with non-zero chunk",
			ExactLocation({ 0, 0, 2 }, { 0.0f, 0.0f, 15.0f }), loc
		);
	}
}

void LocationTest::testAbsolute() {
	{
		RoughLocation loc({ 0, 0, 0 }, { 0, 0, 0 });
		AssertEqual(
			"absolute of rough zero location",
			RoughLocation::Fine(0, 0, 0), loc.Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f });
		AssertEqual(
			"absolute of exact zero location",
			ExactLocation::Fine(0.0f, 0.0f, 0.0f), loc.Absolute()
		);
	}
	{
		RoughLocation loc({ 0, 2, 0 }, { 0, 5, 0 });
		AssertEqual(
			"absolute of rough location",
			RoughLocation::Fine(0, 37, 0), loc.Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 2, 0 }, { 0.0f, 5.0f, 0.0f });
		AssertEqual(
			"absolute of exact location",
			ExactLocation::Fine(0.0f, 37.0f, 0.0f), loc.Absolute()
		);
	}
	{
		RoughLocation loc({ 0, 0, -2 }, { 0, 0, 5 });
		AssertEqual(
			"absolute of rough negative location",
			RoughLocation::Fine(0, 0, -27), loc.Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 0, -2 }, { 0.0f, 0.0f, 5.0f });
		AssertEqual(
			"absolute of exact negative location",
			ExactLocation::Fine(0.0f, 0.0f, -27.0f), loc.Absolute()
		);
	}
}

void LocationTest::testRelative() {
	glm::ivec3 base(0, 0, 0);
	{
		RoughLocation loc({ 0, 0, 0 }, { 0, 0, 0 });
		AssertEqual(
			"relative of rough zero location with zero base",
			RoughLocation::Fine(0, 0, 0), loc.Relative(base).Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f });
		AssertEqual(
			"relative of exact zero location with zero base",
			ExactLocation::Fine(0.0f, 0.0f, 0.0f), loc.Relative(base).Absolute()
		);
	}
	{
		RoughLocation loc({ 0, 2, 0 }, { 0, 5, 0 });
		AssertEqual(
			"relative of rough location with zero base",
			RoughLocation::Fine(0, 37, 0), loc.Relative(base).Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 2, 0 }, { 0.0f, 5.0f, 0.0f });
		AssertEqual(
			"relative of exact location with zero base",
			ExactLocation::Fine(0.0f, 37.0f, 0.0f), loc.Relative(base).Absolute()
		);
	}
	{
		RoughLocation loc({ 0, 0, -2 }, { 0, 0, 5 });
		AssertEqual(
			"relative of rough negative location with zero base",
			RoughLocation::Fine(0, 0, -27), loc.Relative(base).Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 0, -2 }, { 0.0f, 0.0f, 5.0f });
		AssertEqual(
			"relative of exact negative location with zero base",
			ExactLocation::Fine(0.0f, 0.0f, -27.0f), loc.Relative(base).Absolute()
		);
	}

	base = glm::ivec3(0, 1, 0);
	{
		RoughLocation loc({ 0, 0, 0 }, { 0, 0, 0 });
		AssertEqual(
			"relative of rough zero location with positive base",
			RoughLocation::Fine(0, -16, 0), loc.Relative(base).Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f });
		AssertEqual(
			"relative of exact zero location with positive base",
			ExactLocation::Fine(0.0f, -16.0f, 0.0f), loc.Relative(base).Absolute()
		);
	}
	{
		RoughLocation loc({ 0, 2, 0 }, { 0, 5, 0 });
		AssertEqual(
			"relative of rough location with positive base",
			RoughLocation::Fine(0, 21, 0), loc.Relative(base).Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 2, 0 }, { 0.0f, 5.0f, 0.0f });
		AssertEqual(
			"relative of exact location with positive base",
			ExactLocation::Fine(0.0f, 21.0f, 0.0f), loc.Relative(base).Absolute()
		);
	}
	{
		RoughLocation loc({ 0, 0, -2 }, { 0, 0, 5 });
		AssertEqual(
			"relative of rough negative location with positive base",
			RoughLocation::Fine(0, -16, -27), loc.Relative(base).Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 0, -2 }, { 0.0f, 0.0f, 5.0f });
		AssertEqual(
			"relative of exact negative location with positive base",
			ExactLocation::Fine(0.0f, -16.0f, -27.0f), loc.Relative(base).Absolute()
		);
	}

	base = glm::ivec3(-2, 0, 0);
	{
		RoughLocation loc({ 0, 0, 0 }, { 0, 0, 0 });
		AssertEqual(
			"relative of rough zero location with negative base",
			RoughLocation::Fine(32, 0, 0), loc.Relative(base).Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f });
		AssertEqual(
			"relative of exact zero location with negative base",
			ExactLocation::Fine(32.0f, 0.0f, 0.0f), loc.Relative(base).Absolute()
		);
	}
	{
		RoughLocation loc({ 0, 2, 0 }, { 0, 5, 0 });
		AssertEqual(
			"relative of rough location with negative base",
			RoughLocation::Fine(32, 37, 0), loc.Relative(base).Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 2, 0 }, { 0.0f, 5.0f, 0.0f });
		AssertEqual(
			"relative of exact location with negative base",
			ExactLocation::Fine(32.0f, 37.0f, 0.0f), loc.Relative(base).Absolute()
		);
	}
	{
		RoughLocation loc({ 0, 0, -2 }, { 0, 0, 5 });
		AssertEqual(
			"relative of rough negative location with negative base",
			RoughLocation::Fine(32, 0, -27), loc.Relative(base).Absolute()
		);
	}
	{
		ExactLocation loc({ 0, 0, -2 }, { 0.0f, 0.0f, 5.0f });
		AssertEqual(
			"relative of exact negative location with negative base",
			ExactLocation::Fine(32.0f, 0.0f, -27.0f), loc.Relative(base).Absolute()
		);
	}
}

void LocationTest::testDifference() {
	{
		RoughLocation a({ 0, 0, 0 }, { 0, 0, 0 });
		RoughLocation b({ 0, 0, 0 }, { 0, 0, 0 });
		AssertEqual(
			"difference between rough zero locations",
			RoughLocation::Fine(0, 0, 0), a.Difference(b).Absolute()
		);
	}
	{
		ExactLocation a({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f });
		ExactLocation b({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f });
		AssertEqual(
			"difference between exact zero locations",
			ExactLocation::Fine(0.0f, 0.0f, 0.0f), a.Difference(b).Absolute()
		);
	}
	{
		RoughLocation a({ 0, 0, 0 }, { 5, 0, 0 });
		RoughLocation b({ 0, 0, 0 }, { 0, 0, 0 });
		AssertEqual(
			"difference between rough locations",
			RoughLocation::Fine(5, 0, 0), a.Difference(b).Absolute()
		);
	}
	{
		ExactLocation a({ 0, 0, 0 }, { 5.0f, 0.0f, 0.0f });
		ExactLocation b({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f });
		AssertEqual(
			"difference between exact locations",
			ExactLocation::Fine(5.0f, 0.0f, 0.0f), a.Difference(b).Absolute()
		);
	}
	{
		RoughLocation a({ 0, 0, 0 }, { 0, 0, 0 });
		RoughLocation b({ 0, 0, 0 }, { 0, 5, 0 });
		AssertEqual(
			"difference between rough locations",
			RoughLocation::Fine(0, -5, 0), a.Difference(b).Absolute()
		);
	}
	{
		ExactLocation a({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f });
		ExactLocation b({ 0, 0, 0 }, { 0.0f, 5.0f, 0.0f });
		AssertEqual(
			"difference between exact locations",
			ExactLocation::Fine(0.0f, -5.0f, 0.0f), a.Difference(b).Absolute()
		);
	}
	{
		RoughLocation a({ 0, 0, 0 }, { 0, 0, 0 });
		RoughLocation b({ 0, 0, 0 }, { 0, 0, -3 });
		AssertEqual(
			"difference between rough locations",
			RoughLocation::Fine(0, 0, 3), a.Difference(b).Absolute()
		);
	}
	{
		ExactLocation a({ 0, 0, 0 }, { 0.0f, 0.0f, 0.0f });
		ExactLocation b({ 0, 0, 0 }, { 0.0f, 0.0f, -3.0f });
		AssertEqual(
			"difference between exact locations",
			ExactLocation::Fine(0.0f, 0.0f, 3.0f), a.Difference(b).Absolute()
		);
	}
	{
		RoughLocation a({ 1, 0, -1 }, { 5, 14, 9 });
		RoughLocation b({ 0, 2, -1 }, { 3, 2, 0 });
		AssertEqual(
			"difference between rough locations",
			RoughLocation::Fine(18, -20, 9), a.Difference(b).Absolute()
		);
	}
	{
		ExactLocation a({ 1, 0, -1 }, { 5.0f, 14.0f, 9.0f });
		ExactLocation b({ 0, 2, -1 }, { 3.0f, 2.0f, 0.0f });
		AssertEqual(
			"difference between exact locations",
			ExactLocation::Fine(18.0f, -20.0f, 9.0f), a.Difference(b).Absolute()
		);
	}
}


void LocationTest::AssertEqual(
	const std::string &msg,
	const glm::ivec3 &expected,
	const glm::ivec3 &actual
) {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		msg + " X",
		expected.x, actual.x
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		msg + " Y",
		expected.y, actual.y
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		msg + " Z",
		expected.z, actual.z
	);
}

void LocationTest::AssertEqual(
	const std::string &msg,
	const glm::vec3 &expected,
	const glm::vec3 &actual
) {
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg + " X",
		expected.x, actual.x, std::numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg + " Y",
		expected.y, actual.y, std::numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		msg + " Z",
		expected.z, actual.z, std::numeric_limits<float>::epsilon()
	);
}

void LocationTest::AssertEqual(
	const std::string &msg,
	const RoughLocation &expected,
	const RoughLocation &actual
) {
	AssertEqual(
		msg + ": bad chunk",
		expected.chunk, actual.chunk
	);
	AssertEqual(
		msg + ": bad block",
		expected.block, actual.block
	);
}

void LocationTest::AssertEqual(
	const std::string &msg,
	const ExactLocation &expected,
	const ExactLocation &actual
) {
	AssertEqual(
		msg + ": bad chunk",
		expected.chunk, actual.chunk
	);
	AssertEqual(
		msg + ": bad block",
		expected.block, actual.block
	);
}

}
}
