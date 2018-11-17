#ifndef BLANK_GRAPHICS_GL_TRAITS_HPP_
#define BLANK_GRAPHICS_GL_TRAITS_HPP_

#include "glm.hpp"

#include <GL/glew.h>


namespace blank {

template<class T>
struct gl_traits {

	/// number of components per generic attribute
	/// must be 1, 2, 3, 4
	// static constexpr GLint size;

	/// component type
	/// accepted values are:
	///   GL_BYTE, GL_UNSIGNED_BYTE,
	///   GL_SHORT, GL_UNSIGNED_SHORT,
	///   GL_INT, GL_UNSIGNED_INT,
	///   GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE,
	///   GL_FIXED, GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_2_10_10_10_REV
	// static constexpr GLenum type;

};


// basic types

template<> struct gl_traits<signed char> {
	static constexpr GLint size = 1;
	static constexpr GLenum type = GL_BYTE;
};

template<> struct gl_traits<unsigned char> {
	static constexpr GLint size = 1;
	static constexpr GLenum type = GL_UNSIGNED_BYTE;
};

template<> struct gl_traits<short> {
	static constexpr GLint size = 1;
	static constexpr GLenum type = GL_SHORT;
};

template<> struct gl_traits<unsigned short> {
	static constexpr GLint size = 1;
	static constexpr GLenum type = GL_UNSIGNED_SHORT;
};

template<> struct gl_traits<int> {
	static constexpr GLint size = 1;
	static constexpr GLenum type = GL_INT;
};

template<> struct gl_traits<unsigned int> {
	static constexpr GLint size = 1;
	static constexpr GLenum type = GL_UNSIGNED_INT;
};

template<> struct gl_traits<float> {
	static constexpr GLint size = 1;
	static constexpr GLenum type = GL_FLOAT;
};

template<> struct gl_traits<double> {
	static constexpr GLint size = 1;
	static constexpr GLenum type = GL_DOUBLE;
};

// composite types

template<class T, glm::precision P>
struct gl_traits<TVEC1<T, P>> {
	static constexpr GLint size = 1;
	static constexpr GLenum type = gl_traits<T>::type;
};
template<class T, glm::precision P>
constexpr GLint gl_traits<TVEC1<T, P>>::size;
template<class T, glm::precision P>
constexpr GLenum gl_traits<TVEC1<T, P>>::type;

template<class T, glm::precision P>
struct gl_traits<TVEC2<T, P>> {
	static constexpr GLint size = 2;
	static constexpr GLenum type = gl_traits<T>::type;
};
template<class T, glm::precision P>
constexpr GLint gl_traits<TVEC2<T, P>>::size;
template<class T, glm::precision P>
constexpr GLenum gl_traits<TVEC2<T, P>>::type;

template<class T, glm::precision P>
struct gl_traits<TVEC3<T, P>> {
	static constexpr GLint size = 3;
	static constexpr GLenum type = gl_traits<T>::type;
};
template<class T, glm::precision P>
constexpr GLint gl_traits<TVEC3<T, P>>::size;
template<class T, glm::precision P>
constexpr GLenum gl_traits<TVEC3<T, P>>::type;

template<class T, glm::precision P>
struct gl_traits<TVEC4<T, P>> {
	static constexpr GLint size = 4;
	static constexpr GLenum type = gl_traits<T>::type;
};
template<class T, glm::precision P>
constexpr GLint gl_traits<TVEC4<T, P>>::size;
template<class T, glm::precision P>
constexpr GLenum gl_traits<TVEC4<T, P>>::type;

}

#endif
