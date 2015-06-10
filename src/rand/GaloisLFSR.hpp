#ifndef BLANK_RAND_GALOISLFSR_HPP_
#define BLANK_RAND_GALOISLFSR_HPP_

#include <cstdint>
#include <limits>


namespace blank {

class GaloisLFSR {

public:
	// seed should be non-zero
	explicit GaloisLFSR(std::uint64_t seed) noexcept;

	// get the next bit
	bool operator ()() noexcept;

	template<class T>
	T operator ()(T &out) noexcept {
		constexpr int num_bits =
			std::numeric_limits<T>::digits +
			std::numeric_limits<T>::is_signed;
		for (int i = 0; i < num_bits; ++i) {
			operator ()();
		}
		return out = static_cast<T>(state);
	}

private:
	std::uint64_t state;
	// bits 64, 63, 61, and 60 set to 1 (counting from 1 lo to hi)
	static constexpr std::uint64_t mask = 0xD800000000000000;

};
}

#endif
