#ifndef BLANK_APP_PROCESS_HPP_
#define BLANK_APP_PROCESS_HPP_

#include <memory>
#include <string>
#include <vector>


namespace blank {

class Process {

public:
	/// launch process executing the file at given path with
	/// given arguments and environment
	Process(
		const std::string &path,
		const std::vector<std::string> &args,
		const std::vector<std::string> &env);
	~Process();

public:
	/// write to the process' input stream
	/// data is taken from given buffer, at most max_len bytes
	/// @return the number of bytes written
	std::size_t WriteIn(const void *buffer, std::size_t max_len);
	/// read from the process' output stream
	/// data is stored in the given buffer, at most max_len bytes
	/// @return the number of bytes read
	std::size_t ReadOut(void *buffer, std::size_t max_len);
	/// read from the process' error stream
	/// data is stored in the given buffer, at most max_len bytes
	/// @return the number of bytes read
	std::size_t ReadErr(void *buffer, std::size_t max_len);

	/// wait until the process exits and fetch its exit status
	int Join();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;

	bool joined;
	int status;

};

}

#endif
