#ifndef BLANK_GRAPHICS_VERTEXARRAY_HPP_
#define BLANK_GRAPHICS_VERTEXARRAY_HPP_

#include <vector>
#include <GL/glew.h>


namespace blank {

template<std::size_t N>
class VertexArray {

public:
	static constexpr std::size_t NUM_ATTRS = N;

public:
	VertexArray() noexcept;
	~VertexArray() noexcept;

	VertexArray(const VertexArray<N> &) = delete;
	VertexArray<N> &operator =(const VertexArray<N> &) = delete;

	VertexArray(VertexArray<N> &&) noexcept;
	VertexArray<N> &operator =(VertexArray<N> &&) noexcept;

public:
	void Bind() const noexcept;

	template <class T>
	void PushAttribute(std::size_t which, const std::vector<T> &data) noexcept;

	template<class T>
	void PushIndices(std::size_t which, const std::vector<T> &indices) noexcept;

	void DrawLineElements() const noexcept;
	void DrawTriangleElements() const noexcept;

private:
	void BindAttribute(std::size_t which) const noexcept;
	void EnableAttribute(std::size_t which) noexcept;
	template <class T>
	void AttributeData(const std::vector<T> &) noexcept;
	template <class T>
	void AttributePointer(std::size_t which) noexcept;

	void BindIndex(std::size_t which) const noexcept;
	template <class T>
	void IndexData(const std::vector<T> &) noexcept;

private:
	GLuint array_id;
	GLuint attr_id[NUM_ATTRS];

	std::size_t idx_count;
	GLenum idx_type;

};

}

#include "VertexArray.inl"

#endif
