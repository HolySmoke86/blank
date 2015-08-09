#include "io.hpp"

#include <cerrno>
#ifdef _WIN32
#  include <direct.h>
#endif
#include <sys/stat.h>


namespace blank {

bool is_dir(const char *path) {
#ifdef _WIN32
	struct _stat info;
	if (_stat(path, &info) != 0) {
		return false;
	}
	return (info.st_mode & _S_IFDIR) != 0;
#else
	struct stat info;
	if (stat(path, &info) != 0) {
		return false;
	}
	return S_ISDIR(info.st_mode);
#endif
}

bool is_file(const char *path) {
#ifdef _WIN32
	struct _stat info;
	if (_stat(path, &info) != 0) {
		return false;
	}
	return (info.st_mode & _S_IFREG) != 0;
#else
	struct stat info;
	if (stat(path, &info) != 0) {
		return false;
	}
	return S_ISREG(info.st_mode);
#endif
}


bool make_dir(const char *path) {
#ifdef _WIN32
	int ret = _mkdir(path);
#else
	int ret = mkdir(path, 0777);
#endif
	return ret == 0;
}


bool make_dirs(const std::string &path) {
	if (make_dir(path)) {
		return true;
	}

	switch (errno) {

		case ENOENT:
			// missing component
			{
#ifdef _WIN32
				auto pos = path.find_last_of("\\/");
#else
				auto pos = path.find_last_of('/');
#endif
				if (pos == std::string::npos) {
					return false;
				}
				if (pos == path.length() - 1) {
					// trailing separator, would make final make_dir fail
#ifdef _WIN32
					 pos = path.find_last_of("\\/", pos - 1);
#else
					 pos = path.find_last_of('/', pos - 1);
#endif
					if (pos == std::string::npos) {
						return false;
					}
				}
				if (!make_dirs(path.substr(0, pos))) {
					return false;
				}
			}
			// try again
			return make_dir(path);

		case EEXIST:
			// something's there, check if it's a dir and we're good
			return is_dir(path);

		default:
			// whatever else went wrong, it can't be good
			return false;

	}
}

}
