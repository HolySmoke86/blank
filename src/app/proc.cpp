#include "Process.hpp"

#ifdef __WIN32
#  error "TODO: windows implementation of Process"
#else
#  include <cstdio>
#  include <fcntl.h>
#  include <unistd.h>
#  include <sys/wait.h>
#endif

#include <stdexcept>

using namespace std;


namespace blank {

struct Process::Impl {

	Impl(
		const string &path_in,
		const vector<string> &args,
		const vector<string> &env);
	~Impl();

	size_t WriteIn(const void *buffer, size_t max_len);
	size_t ReadOut(void *buffer, size_t max_len);
	size_t ReadErr(void *buffer, size_t max_len);

	int Join();

#ifdef __WIN32
	PROCESS_INFORMATION pi;
	HANDLE fd_in[2];
	HANDLE fd_out[2];
	HANDLE fd_err[2];
#else
	int pid;
	int fd_in[2];
	int fd_out[2];
	int fd_err[2];
#endif

};


Process::Process(
	const string &path,
	const vector<string> &args,
	const vector<string> &env)
: impl(new Impl(path, args, env))
, joined(false)
, status(0) {

}

Process::~Process() {
	Join();
}


size_t Process::WriteIn(const void *buffer, size_t max_len) {
	return impl->WriteIn(buffer, max_len);
}

size_t Process::ReadOut(void *buffer, size_t max_len) {
	return impl->ReadOut(buffer, max_len);
}

size_t Process::ReadErr(void *buffer, size_t max_len) {
	return impl->ReadErr(buffer, max_len);
}

int Process::Join() {
	if (joined) {
		return status;
	} else {
		status = impl->Join();
		joined = true;
		return status;
	}
}


Process::Impl::Impl(
	const string &path_in,
	const vector<string> &args,
	const vector<string> &env
) {
	const char *path = path_in.c_str();
	char *envp[env.size() + 1];
	for (size_t i = 0; i < env.size(); ++i) {
		envp[i] = const_cast<char *>(env[i].c_str());
	}
	envp[env.size()] = nullptr;
#ifdef __WIN32
	string cmdline;
	for (const auto &arg : args) {
		cmdline += '"';
		cmdline += arg;
		cmdline += '"';
	}

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = true;
	sa.lpSecurityDescriptor = nullptr;
	if (!CreatePipe(&fd_in[0], &fd_in[1], &sa, 0)) {
		throw runtime_error("failed to open pipe for child process' stdin");
	}
	if (!SetHandleInformation(fd_in[1], HANDLE_FLAG_INHERIT, 0)) {
		throw runtime_error("failed to stop child process from inheriting stdin write handle");
	}
	if (!CreatePipe(&fd_out[0], &fd_out[1], &sa, 0)) {
		throw runtime_error("failed to open pipe for child process' stdout");
	}
	if (!SetHandleInformation(fd_out[0], HANDLE_FLAG_INHERIT, 0)) {
		throw runtime_error("failed to stop child process from inheriting stdout read handle");
	}
	if (!CreatePipe(&fd_err[0], &fd_err[1], &sa, 0)) {
		throw runtime_error("failed to open pipe for child process' stderr");
	}
	if (!SetHandleInformation(fd_err[0], HANDLE_FLAG_INHERIT, 0)) {
		throw runtime_error("failed to stop child process from inheriting stderr read handle");
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.hStdError = fd_err[1];
	si.hStdOutput = fd_out[1];
	si.hStdInput = fd_in[0];
	si.dwFlags |= STARTF_USESTDHANDLES;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	if (!CreateProcess(
		path,
		cmdline.c_str(),
		nullptr,
		nullptr,
		true,
		0,
		envp,
		nullptr,
		&si,
		&pi
	)) {
		throw runtime_error("CreateProcess");
	}
#else
	char *argv[args.size() + 1];
	for (size_t i = 0; i < args.size(); ++i) {
		// I was promised exec won't modify my characters
		argv[i] = const_cast<char *>(args[i].c_str());
	}
	argv[args.size()] = nullptr;

	if (pipe(fd_in) != 0) {
		throw runtime_error("failed to open pipe for child process' stdin");
	}
	if (pipe(fd_out) != 0) {
		throw runtime_error("failed to open pipe for child process' stdout");
	}
	if (pipe(fd_err) != 0) {
		throw runtime_error("failed to open pipe for child process' stderr");
	}

	pid = fork();
	if (pid == -1) {
		throw runtime_error("fork");
	} else if (pid == 0) {

		if (dup2(fd_in[0], STDIN_FILENO) == -1) {
			exit(EXIT_FAILURE);
		}
		if (dup2(fd_out[1], STDOUT_FILENO) == -1) {
			exit(EXIT_FAILURE);
		}
		if (dup2(fd_err[1], STDERR_FILENO) == -1) {
			exit(EXIT_FAILURE);
		}

		close(fd_in[0]);
		close(fd_in[1]);
		close(fd_out[0]);
		close(fd_out[1]);
		close(fd_err[0]);
		close(fd_err[1]);

		execve(path, argv, envp);
		// if execve returns, something bad happened
		exit(EXIT_FAILURE);

	} else {

		close(fd_in[0]);
		close(fd_out[1]);
		close(fd_err[1]);

	}
}
#endif

Process::Impl::~Impl() {

}

size_t Process::Impl::WriteIn(const void *buffer, size_t max_len) {
#ifdef __WIN32
	DWORD written;
	if (!WriteFile(fd_in[1], buffer, max_len, &written, nullptr)) {
		throw runtime_error("failed to write to child process' input stream");
	}
	return written;
#else
	int written = write(fd_in[1], buffer, max_len);
	if (written < 0) {
		if (errno == EAGAIN) {
			return 0;
		} else {
			throw runtime_error("failed to write to child process' input stream");
		}
	}
	return written;
#endif
}

size_t Process::Impl::ReadOut(void *buffer, size_t max_len) {
#ifdef __WIN32
	DWORD ret;
	if (!ReadFile(fd_out[0], buffer, max_len, &ret, nullptr)) {
		throw runtime_error("failed to read from child process' output stream");
	}
	return ret;
#else
	int ret = read(fd_out[0], buffer, max_len);
	if (ret < 0) {
		if (errno == EAGAIN) {
			return 0;
		} else {
			throw runtime_error("failed to read from child process' output stream");
		}
	}
	return ret;
#endif
}

size_t Process::Impl::ReadErr(void *buffer, size_t max_len) {
#ifdef __WIN32
	DWORD ret;
	if (!ReadFile(fd_err[0], buffer, max_len, &ret, nullptr)) {
		throw runtime_error("failed to read from child process' error stream");
	}
	return ret;
#else
	int ret = read(fd_err[0], buffer, max_len);
	if (ret < 0) {
		if (errno == EAGAIN) {
			return 0;
		} else {
			throw runtime_error("failed to read from child process' error stream");
		}
	}
	return ret;
#endif
}

int Process::Impl::Join() {
#ifdef __WIN32
	CloseHandle(fd_in[1]);
	CloseHandle(fd_out[0]);
	CloseHandle(fd_err[0]);

	DWORD exit_code;
	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &exit_code);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exit_code;
#else
	// close streams before waiting on child termination
	close(fd_in[1]);
	close(fd_out[0]);
	close(fd_err[0]);

	while (true) {
		int status;
		int result = waitpid(pid, &status, 0);
		if (result == -1) {
			throw runtime_error("error waiting on child process");
		}
		if (result == pid && WIFEXITED(status)) {
			return WEXITSTATUS(status);
		}
		// otherwise, child probably signalled, which we don't care
		// about (please don't tell youth welfare), so try again
	}
#endif
}

}
