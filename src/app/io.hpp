#ifndef BLANK_APP_IO_HPP_
#define BLANK_APP_IO_HPP_

#include <string>


namespace blank {

/// check if give path points to an existing directory
bool is_dir(const char *);

/// create given directory
/// @return true if the directory was created
///         the directory might already exist, see errno
bool make_dir(const char *);
/// create given directory and all parents
/// @return true if the directory was created or already exists
bool make_dirs(const std::string &);

}

#endif
