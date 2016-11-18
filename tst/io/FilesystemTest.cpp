#include "FilesystemTest.hpp"

#include "io/filesystem.hpp"

#include <algorithm>

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::FilesystemTest);

using namespace std;


namespace blank {
namespace test {

void FilesystemTest::setUp() {
	test_dir.reset(new TempDir());
}

void FilesystemTest::tearDown() {
	test_dir.reset();
}


void FilesystemTest::testFile() {
#ifdef _WIN32
	const string test_file = test_dir->Path() + "\\test-file.txt";
#else
	const string test_file = test_dir->Path() + "/test-file";
#endif

	CPPUNIT_ASSERT_MESSAGE(
		"inexistant file is file",
		!is_file(test_file));
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"mtime of inexistant file should be zero",
		time_t(0), file_mtime(test_file));
	CPPUNIT_ASSERT_MESSAGE(
		"inexistant file is a directory",
		!is_dir(test_file));

	{ // create file
		ofstream file(test_file);
		file << "hello" << endl;
	}
	time_t now = time(nullptr);
	CPPUNIT_ASSERT_MESSAGE(
		"existing file not a file",
		is_file(test_file));
	CPPUNIT_ASSERT_MESSAGE(
		"mtime of existing file should be somewhere around now",
		// let's assume that creating the file takes less than five seconds
		abs(now - file_mtime(test_file) < 5));
	CPPUNIT_ASSERT_MESSAGE(
		"regular file is a directory",
		!is_dir(test_file));

	CPPUNIT_ASSERT_MESSAGE(
		"failed to remove test file",
		remove_file(test_file));

	CPPUNIT_ASSERT_MESSAGE(
		"removed file is still a file",
		!is_file(test_file));
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"mtime of removed file should be zero",
		time_t(0), file_mtime(test_file));
	CPPUNIT_ASSERT_MESSAGE(
		"removed file became a directory",
		!is_dir(test_file));
}

void FilesystemTest::testDirectory() {
#ifdef _WIN32
	const string test_subdir = test_dir->Path() + "\\a";
	const string test_subsubdir = test_subdir + "\\b";
	const string test_file = test_subsubdir + "\\c.txt";
#else
	const string test_subdir = test_dir->Path() + "/a";
	const string test_subsubdir = test_subdir + "/b";
	const string test_file = test_subsubdir + "/c";
#endif

	CPPUNIT_ASSERT_MESSAGE(
		"inexistant directory is a file",
		!is_file(test_subdir));
	CPPUNIT_ASSERT_MESSAGE(
		"inexistant directory is a directory",
		!is_dir(test_subdir));

	CPPUNIT_ASSERT_MESSAGE(
		"failed to create test subdir",
		make_dir(test_subdir));
	CPPUNIT_ASSERT_MESSAGE(
		"created directory is a file",
		!is_file(test_subdir));
	CPPUNIT_ASSERT_MESSAGE(
		"created directory is not a directory",
		is_dir(test_subdir));

	CPPUNIT_ASSERT_MESSAGE(
		"failed to remove test subdir",
		remove_dir(test_subdir));
	CPPUNIT_ASSERT_MESSAGE(
		"removed directory became a file",
		!is_file(test_subdir));
	CPPUNIT_ASSERT_MESSAGE(
		"removed directory is still a directory",
		!is_dir(test_subdir));

	CPPUNIT_ASSERT_MESSAGE(
		"failed to create test subdirs",
		make_dirs(test_subsubdir));
	CPPUNIT_ASSERT_MESSAGE(
		"created directory is a file",
		!is_file(test_subdir));
	CPPUNIT_ASSERT_MESSAGE(
		"created directory is not a directory",
		is_dir(test_subdir));
	CPPUNIT_ASSERT_MESSAGE(
		"created directory is a file",
		!is_file(test_subsubdir));
	CPPUNIT_ASSERT_MESSAGE(
		"created directory is not a directory",
		is_dir(test_subsubdir));

	{ // create file
		ofstream file(test_file);
		file << "hello" << endl;
	}
	CPPUNIT_ASSERT_MESSAGE(
		"failed to create test file",
		is_file(test_file));

	CPPUNIT_ASSERT_MESSAGE(
		"failed to remove test subdir",
		remove_dir(test_subdir));
	CPPUNIT_ASSERT_MESSAGE(
		"removed directory became a file",
		!is_file(test_subdir));
	CPPUNIT_ASSERT_MESSAGE(
		"removed directory is still a directory",
		!is_dir(test_subdir));
	CPPUNIT_ASSERT_MESSAGE(
		"removed directory became a file",
		!is_file(test_subsubdir));
	CPPUNIT_ASSERT_MESSAGE(
		"removed directory is still a directory",
		!is_dir(test_subsubdir));
	CPPUNIT_ASSERT_MESSAGE(
		"removed file became a directory",
		!is_dir(test_file));
	CPPUNIT_ASSERT_MESSAGE(
		"removed file is still a file",
		!is_file(test_file));
}

}
}
