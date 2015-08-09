#ifndef BLANK_APP_IO_HPP_
#define BLANK_APP_IO_HPP_

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

}

#endif
