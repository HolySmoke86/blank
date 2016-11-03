#ifndef BLANK_GEOMETRY_LOCATION_HPP_
#define BLANK_GEOMETRY_LOCATION_HPP_

#include "../graphics/glm.hpp"


namespace blank {

template<class T>
struct Location {

	using Coarse = glm::ivec3;
	using CoarseScalar = int;
	using Fine = TVEC3<T, glm::precision(0)>;
	using FineScalar = T;
	using Self = Location<T>;


	Coarse chunk;
	Fine block;


	/// scale of coarse vs fine coordinates
	static constexpr CoarseScalar scale = 16;
	/// scale with same type as fine
	static constexpr FineScalar fscale = scale;
	/// scale in three dimensions
	static Coarse Extent() noexcept { return Coarse(scale); }
	/// extent with same type as fine
	static Fine FExtent() noexcept { return Fine(fscale); }


	Location() noexcept : chunk(CoarseScalar(0)), block(FineScalar(0)) { }
	Location(const Coarse &c, const Fine &b) noexcept : chunk(c), block(b) { }

	/// from absolute position
	/// not sanitized
	explicit Location(const Fine &b) noexcept : chunk(CoarseScalar(0)), block(b) { }

	/// make sure fine coordinates are within [0,scale)
	/// use this if block isn't too far out of range
	Self &Correct() noexcept;
	/// use this if block is way out of range
	Self &Sanitize() noexcept;

	/// resolve absolute position
	Fine Absolute() const noexcept {
		return Fine(chunk * Extent()) + block;
	}
	/// get location relative to given coarse coordinates
	Self Relative(const Coarse &reference) const noexcept {
		return Self(chunk - reference, block);
	}
	/// get difference between this and given location
	/// (= this - given, points from given to this)
	/// returned location is not sanitized
	Self Difference(const Location &other) const noexcept {
		return Self(chunk - other.chunk, block - other.block);
	}

};

template<class T> constexpr typename Location<T>::CoarseScalar Location<T>::scale;
template<class T> constexpr typename Location<T>::FineScalar Location<T>::fscale;

template<class T>
inline Location<T> &Location<T>::Correct() noexcept {
	while (block.x >= fscale) {
		block.x -= fscale;
		++chunk.x;
	}
	while (block.x < 0) {
		block.x += fscale;
		--chunk.x;
	}
	while (block.y >= fscale) {
		block.y -= fscale;
		++chunk.y;
	}
	while (block.y < 0) {
		block.y += fscale;
		--chunk.y;
	}
	while (block.z >= fscale) {
		block.z -= fscale;
		++chunk.z;
	}
	while (block.z < 0) {
		block.z += fscale;
		--chunk.z;
	}
	return *this;
}

template<class T>
inline Location<T> &Location<T>::Sanitize() noexcept {
	Coarse diff = Coarse(block) / Extent();
	chunk += diff;
	block -= diff * Extent();
	// may leave negative coordinates in block
	return Correct();
}

using ExactLocation = Location<float>;
using RoughLocation = Location<int>;

}

#endif
