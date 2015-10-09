#include "filesystem.hpp"

#include <cerrno>
#ifdef _WIN32
#  include <direct.h>
#endif
#include <sys/stat.h>


namespace blank {

namespace {
#ifdef _WIN32
	using Stat = struct _stat;
	int do_stat(const char *path, Stat &info) {
		return _stat(path, &info);
	}
	bool is_dir(const Stat &info) {
		return (info.st_mode & _S_IFDIR) != 0;
	}
	bool is_file(const Stat &info) {
		return (info.st_mode & _S_IFEG) != 0;
	}
#else
	using Stat = struct stat;
	int do_stat(const char *path, Stat &info) {
		return stat(path, &info);
	}
	bool is_dir(const Stat &info) {
		return S_ISDIR(info.st_mode);
	}
	bool is_file(const Stat &info) {
		return S_ISREG(info.st_mode);
	}
#endif
	std::time_t get_mtime(const Stat &info) {
#ifdef __APPLE__
		return info.st_mtimespec.tv_sec;
#else
	return info.st_mtime;
#endif
	}
}

bool is_dir(const char *path) {
	Stat info;
	if (do_stat(path, info) != 0) {
		return false;
	}
	return is_dir(info);
}

bool is_file(const char *path) {
	Stat info;
	if (do_stat(path, info) != 0) {
		return false;
	}
	return is_file(info);
}

std::time_t file_mtime(const char *path) {
	Stat info;
	if (do_stat(path, info) != 0) {
		return 0;
	}
	return get_mtime(info);
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
