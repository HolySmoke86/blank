#include "../graphics/gl_traits.hpp"

namespace blank {

template<std::size_t N>
VertexArray<N>::VertexArray() noexcept
: idx_count(0)
, idx_type(GL_UNSIGNED_INT) {
	glGenVertexArrays(1, &array_id);
	glGenBuffers(N, attr_id);
}

template<std::size_t N>
VertexArray<N>::~VertexArray() noexcept {
	if (array_id != 0) {
		glDeleteBuffers(N, attr_id);
		glDeleteVertexArrays(1, &array_id);
	}
}

template<std::size_t N>
VertexArray<N>::VertexArray(VertexArray<N> &&other) noexcept
: array_id(other.array_id)
, idx_count(other.idx_count)
, idx_type(other.idx_type) {
	other.array_id = 0;
	for (std::size_t i = 0; i < N; ++i) {
		attr_id[i] = other.attr_id[i];
		other.attr_id[i] = 0;
	}
}

template<std::size_t N>
VertexArray<N> &VertexArray<N>::operator =(VertexArray<N> &&other) noexcept {
	std::swap(array_id, other.array_id);
	for (std::size_t i = 0; i < N; ++i) {
		std::swap(attr_id[i], other.attr_id[i]);
	}
	idx_count = other.idx_count;
	idx_type = other.idx_type;
	return *this;
}

template<std::size_t N>
void VertexArray<N>::Bind() const noexcept {
	glBindVertexArray(array_id);
}

template<std::size_t N>
template <class T>
void VertexArray<N>::PushAttribute(std::size_t which, const std::vector<T> &data) noexcept {
	BindAttribute(which);
	AttributeData(data);
	EnableAttribute(which);
	AttributePointer<T>(which);
}

template<std::size_t N>
void VertexArray<N>::BindAttribute(std::size_t i) const noexcept {
	assert(i < NUM_ATTRS && "vertex attribute ID out of bounds");
	glBindBuffer(GL_ARRAY_BUFFER, attr_id[i]);
}

template<std::size_t N>
void VertexArray<N>::EnableAttribute(std::size_t i) noexcept {
	assert(i < NUM_ATTRS && "vertex attribute ID out of bounds");
	glEnableVertexAttribArray(i);
}

template<std::size_t N>
template<class T>
void VertexArray<N>::AttributeData(const std::vector<T> &buf) noexcept {
	glBufferData(GL_ARRAY_BUFFER, buf.size() * sizeof(T), buf.data(), GL_STATIC_DRAW);
}

template<std::size_t N>
template <class T>
void VertexArray<N>::AttributePointer(std::size_t which) noexcept {
	glVertexAttribPointer(
		which,              // program location
		gl_traits<T>::size, // element size
		gl_traits<T>::type, // element type
		GL_FALSE,           // normalize to [-1,1] or [0,1] for unsigned types
		0,                  // stride
		nullptr             // offset
	);
}

template<std::size_t N>
template <class T>
void VertexArray<N>::PushIndices(std::size_t which, const std::vector<T> &indices) noexcept {
	BindIndex(which);
	IndexData(indices);
}

template<std::size_t N>
void VertexArray<N>::BindIndex(std::size_t i) const noexcept {
	assert(i < NUM_ATTRS && "element index ID out of bounds");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, attr_id[i]);
}

template<std::size_t N>
template<class T>
void VertexArray<N>::IndexData(const std::vector<T> &buf) noexcept {
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf.size() * sizeof(T), buf.data(), GL_STATIC_DRAW);
	idx_count = buf.size();
	idx_type = gl_traits<T>::type;
}


template<std::size_t N>
void VertexArray<N>::DrawLineElements() const noexcept {
	Bind();
	glDrawElements(
		GL_LINES,  // how
		idx_count, // count
		idx_type,  // type
		nullptr    // offset
	);
}

template<std::size_t N>
void VertexArray<N>::DrawTriangleElements() const noexcept {
	Bind();
	glDrawElements(
		GL_TRIANGLES, // how
		idx_count,    // count
		idx_type,     // type
		nullptr       // offset
	);
}

}
