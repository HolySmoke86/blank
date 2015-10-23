#ifndef BLANK_RAND_GALOISLFSR_HPP_
#define BLANK_RAND_GALOISLFSR_HPP_

#include <cstdint>
#include <limits>


namespace blank {

class GaloisLFSR {

public:
	// seed should be non-zero
	explicit GaloisLFSR(std::uint64_t seed) noexcept
	: state(seed) {
		if (state == 0) {
			state = 1;
		}
	}

	// get the next bit
	bool operator ()() noexcept {
		bool result = state & 1;
		state >>= 1;
		if (result) {
			state |= 0x8000000000000000;
			state ^= mask;
		} else {
			state &= 0x7FFFFFFFFFFFFFFF;
		}
		return result;
	}

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

	template<class T>
	T Next() noexcept {
		T next;
		return (*this)(next);
	}

	float SNorm() noexcept {
		return float(Next<std::uint32_t>()) * (1.0f / 2147483647.5f) - 1.0f;
	}

	float UNorm() noexcept {
		return float(Next<std::uint32_t>()) * (1.0f / 4294967295.0f);
	}

	template<class Container>
	typename Container::reference From(Container &c) {
		return c[Next<typename Container::size_type>() % c.size()];
	}
	template<class Container>
	typename Container::const_reference From(const Container &c) {
		return c[Next<typename Container::size_type>() % c.size()];
	}

private:
	std::uint64_t state;
	// bits 64, 63, 61, and 60 set to 1 (counting from 1 lo to hi)
	static constexpr std::uint64_t mask = 0xD800000000000000;

};

}

#endif
