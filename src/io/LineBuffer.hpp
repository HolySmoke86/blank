#ifndef BLANK_IO_LINEBUFFER_HPP_
#define BLANK_IO_LINEBUFFER_HPP_

#include <algorithm>
#include <cassert>
#include <string>


namespace blank {

template<std::size_t size>
class LineBuffer {

public:
	explicit LineBuffer(char term = '\n') noexcept
	: buffer{0}
	, term(term)
	, head(buffer) { }

	char *begin() noexcept {
		return buffer;
	}
	const char *begin() const noexcept {
		return buffer;
	}
	char *end() noexcept {
		return head;
	}
	const char *end() const noexcept {
		return head;
	}

	/// extract one line from the buffer, terminator not included
	/// @return false if the buffer does not contain a complete line
	bool Extract(std::string &line) {
		char *line_end = std::find(begin(), end(), term);
		if (line_end == end()) {
			return false;
		}
		line.assign(begin(), line_end);
		++line_end;
		std::move(line_end, end(), begin());
		head -= std::distance(begin(), line_end);
		return true;
	}

	/// get a pointer to append data to the buffer
	/// it is safe to write at most Remain() bytes
	char *WriteHead() noexcept {
		return head;
	}

	// call when data has been written to WriteHead()
	void Update(std::size_t written) {
		assert(written <= Remain());
		head += written;
	}

	std::size_t Remain() const noexcept {
		return std::distance(end(), buffer + size);
	}

private:
	char buffer[size];
	char term;
	char *head;

};

}

#endif
