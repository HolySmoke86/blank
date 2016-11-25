#ifndef BLANK_APP_PROCESS_HPP_
#define BLANK_APP_PROCESS_HPP_

#include <memory>
#include <string>
#include <vector>


namespace blank {

class Process {

public:
	using Arguments = std::vector<std::string>;
	using Environment = std::vector<std::string>;

public:
	/// launch process executing the file at given path with
	/// given arguments and environment of parent process
	Process(
		const std::string &path,
		const Arguments &args);
	/// launch process executing the file at given path with
	/// given arguments and given environment
	Process(
		const std::string &path,
		const Arguments &args,
		const Environment &env);
	~Process();

public:
	/// write to the process' input stream
	/// data is taken from given buffer, at most max_len bytes
	/// @return the number of bytes written
	std::size_t WriteIn(const void *buffer, std::size_t max_len);
	/// close program's input stream
	void CloseIn();

	/// read from the process' output stream
	/// data is stored in the given buffer, at most max_len bytes
	/// timeout is the number of milliseconds to wait for the process
	/// to produce output, -1 for indefinite
	/// @return the number of bytes read
	std::size_t ReadOut(void *buffer, std::size_t max_len, int timeout);
	/// close program's output stream
	void CloseOut();

	/// read from the process' error stream
	/// data is stored in the given buffer, at most max_len bytes
	/// timeout is the number of milliseconds to wait for the process
	/// to produce output, -1 for indefinite
	/// @return the number of bytes read
	std::size_t ReadErr(void *buffer, std::size_t max_len, int timeout);
	/// close program's output stream
	void CloseErr();

	/// ask the process nicely to terminate
	/// (except on win32)
	void Terminate();
	/// check if the process has terminated
	bool Terminated();
	/// wait until the process exits and fetch its exit status
	int Join();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;

};

}

#endif
