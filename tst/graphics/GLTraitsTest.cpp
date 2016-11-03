#include "GLTraitsTest.hpp"

#include "graphics/gl_traits.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::GLTraitsTest);


namespace blank {
namespace test {

void GLTraitsTest::setUp() {

}

void GLTraitsTest::tearDown() {

}


void GLTraitsTest::testSize() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for byte",
		1, gl_traits<signed char>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for ubyte",
		1, gl_traits<unsigned char>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for short",
		1, gl_traits<short>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for ushort",
		1, gl_traits<unsigned short>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for int",
		1, gl_traits<int>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for uint",
		1, gl_traits<unsigned int>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for float",
		1, gl_traits<float>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for double",
		1, gl_traits<double>::size
	);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for vec2",
		2, gl_traits<glm::vec2>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for vec3",
		3, gl_traits<glm::vec3>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for vec4",
		4, gl_traits<glm::vec4>::size
	);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for vec2i",
		2, gl_traits<glm::ivec2>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for vec3i",
		3, gl_traits<glm::ivec3>::size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad number of components for vec4i",
		4, gl_traits<glm::ivec4>::size
	);
}

void GLTraitsTest::testType() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for byte",
		GLenum(GL_BYTE), gl_traits<signed char>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for ubyte",
		GLenum(GL_UNSIGNED_BYTE), gl_traits<unsigned char>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for short",
		GLenum(GL_SHORT), gl_traits<short>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for ushort",
		GLenum(GL_UNSIGNED_SHORT), gl_traits<unsigned short>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for int",
		GLenum(GL_INT), gl_traits<int>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for uint",
		GLenum(GL_UNSIGNED_INT), gl_traits<unsigned int>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for float",
		GLenum(GL_FLOAT), gl_traits<float>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for double",
		GLenum(GL_DOUBLE), gl_traits<double>::type
	);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for vec2",
		GLenum(GL_FLOAT), gl_traits<glm::vec2>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for vec3",
		GLenum(GL_FLOAT), gl_traits<glm::vec3>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for vec4",
		GLenum(GL_FLOAT), gl_traits<glm::vec4>::type
	);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for vec2i",
		GLenum(GL_INT), gl_traits<glm::ivec2>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for vec3i",
		GLenum(GL_INT), gl_traits<glm::ivec3>::type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad component type for vec4i",
		GLenum(GL_INT), gl_traits<glm::ivec4>::type
	);
}

}
}
