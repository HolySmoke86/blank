#include "filesystem.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#ifdef _WIN32
#  include <conio.h>
#  include <direct.h>
#  include <windows.h>
#else
#  include <dirent.h>
#  include <sys/types.h>
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


bool remove_file(const std::string &path) {
	return remove(path.c_str()) == 0;
}


bool remove_dir(const std::string &path) {
#ifdef _WIN32

	// shamelessly stolen from http://www.codeguru.com/forum/showthread.php?t=239271
	const std::string pattern = path + "\\*.*";
	WIN32_FIND_DATA info;
	HANDLE file = FindFirstFile(pattern.c_str(), &info);
	if (file == INVALID_HANDLE_VALUE) {
		// already non-existing
		return true;
	}

	do {
		if (
			strncmp(info.cFileName, ".", 2) == 0 ||
			strncmp(info.cFileName, "..", 3) == 0
		) {
			continue;
		}
		const std::string sub_path = path + '\\' + info.cFileName;
		if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			if (!remove_dir(sub_path)) {
				return false;
			}
		} else {
			if (!SetFileAttributes(sub_path.c_str(), FILE_ATTRIBUTE_NORMAL)) {
				return false;
			}
			if (!remove_file(sub_path)) {
				return false;
			}
		}
	} while (FindNextFile(file, &info));
	FindClose(file);

	DWORD error = GetLastError();
	if (error != ERROR_NO_MORE_FILES) {
		return false;
	}
	// is this (NORMAL vs DIRECTORY) really correct?
	if (!SetFileAttributes(path.c_str(), FILE_ATTRIBUTE_NORMAL)) {
		return false;
	}
	return RemoveDirectory(path.c_str());

#else

	DIR *dir = opendir(path.c_str());
	for (dirent *entry = readdir(dir); entry != nullptr; entry = readdir(dir)) {
		if (
			strncmp(entry->d_name, ".", 2) == 0 ||
			strncmp(entry->d_name, "..", 3) == 0
		) {
			continue;
		}
		const std::string sub_path = path + '/' + entry->d_name;
		if (is_dir(sub_path)) {
			if (!remove_dir(sub_path)) {
				return false;
			}
		} else {
			if (!remove_file(sub_path)) {
				return false;
			}
		}
	}
	return remove(path.c_str()) == 0;

#endif
}

}
