#ifndef BLANK_IO_FILESYSTEM_HPP_
#define BLANK_IO_FILESYSTEM_HPP_

#include <ctime>
#include <string>


namespace blank {

/// check if give path points to an existing directory
bool is_dir(const char *);
inline bool is_dir(const std::string &s) {
	return is_dir(s.c_str());
}
/// check if give path points to an existing file
bool is_file(const char *);
inline bool is_file(const std::string &s) {
	return is_file(s.c_str());
}
/// get timestamp of last modification
std::time_t file_mtime(const char *);
inline std::time_t file_mtime(const std::string &s) {
	return file_mtime(s.c_str());
}

/// create given directory
/// @return true if the directory was created
///         the directory might already exist, see errno
bool make_dir(const char *);
inline bool make_dir(const std::string &s) {
	return make_dir(s.c_str());
}
/// create given directory and all parents
/// @return true if the directory was created or already exists
bool make_dirs(const std::string &);

/// remove given file
/// @return true on success
bool remove_file(const std::string &);
/// recursively remove given directory
/// may leave the directory partially removed on failure
/// @return true if the directory was completely removed
bool remove_dir(const std::string &);


/// Create a temporary directory with lifetime tie to the instance's.
/// Note that the directory may survive its object if removal fails
/// for any reason, e.g. another process changing permissions.
class TempDir {

public:
	TempDir();
	~TempDir();

	TempDir(const TempDir &) = delete;
	TempDir &operator =(const TempDir &) = delete;

public:
	const std::string &Path() const noexcept { return path; }

private:
	std::string path;

};

}

#endif
