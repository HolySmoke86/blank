#include "gl_traits.hpp"


namespace blank {

constexpr GLint gl_traits<signed char>::size;
constexpr GLenum gl_traits<signed char>::type;

constexpr GLint gl_traits<unsigned char>::size;
constexpr GLenum gl_traits<unsigned char>::type;

constexpr GLint gl_traits<short>::size;
constexpr GLenum gl_traits<short>::type;

constexpr GLint gl_traits<unsigned short>::size;
constexpr GLenum gl_traits<unsigned short>::type;

constexpr GLint gl_traits<int>::size;
constexpr GLenum gl_traits<int>::type;

constexpr GLint gl_traits<unsigned int>::size;
constexpr GLenum gl_traits<unsigned int>::type;

constexpr GLint gl_traits<float>::size;
constexpr GLenum gl_traits<float>::type;

constexpr GLint gl_traits<double>::size;
constexpr GLenum gl_traits<double>::type;

}
