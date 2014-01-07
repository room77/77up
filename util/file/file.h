// Copyright 2010-2013 Optrip, Room77
// Author: Calvin Yang, Pramod Gupta, Uygar Oztekin

// File-related utilities.

#ifndef _PUBLIC_UTIL_FILE_FILE_H_
#define _PUBLIC_UTIL_FILE_FILE_H_

#include <unistd.h>
#include <sys/stat.h>

#include <fstream>

#include "base/defs.h"
#include "base/logging.h"
#include "util/string/strutil.h"

// a collection of file processing functions

namespace file {

inline bool Exists(const string& filename, struct stat* buf) {
  return (stat(filename.c_str(), buf) == 0);
}

inline bool Exists(const string& filename) {
  struct stat buf;
  return Exists(filename, &buf);
}

// We treat everything that is not a directory as a file.
// We may need to narrow this definition in future, but for now this is
// sufficient.
inline bool IsFile(const string& filename) {
  struct stat buf;
  if (Exists(filename, &buf)) return !(S_ISDIR(buf.st_mode));
  return false;  // file not found -- return "not a file"
}

// Returns true if passed filename is a directory, false otherwise.
// Returns false if no file/directory with the name exists.
inline bool IsDirectory(const string& filename) {
  struct stat buf;
  if (Exists(filename, &buf)) return S_ISDIR(buf.st_mode);
  return false;  // file not found -- return "not a directory"
}

inline bool FileModifiedSince(const string& filename, time_t t) {
  struct stat buf;
  if (Exists(filename, &buf)) {
    // file modified since time t, and it has been at least 2 seconds since
    // the most recent modification (this is to prevent reading partial files
    // while the file is being updated)
    return (buf.st_mtime >= t && (time(NULL) - buf.st_mtime > 2));
  }
  return false;  // file not found -- return "not modified"
}

// given a file pattern, return all matching files
int GetMatchingFiles(const char *pattern, vector<string> *result);
inline int GetMatchingFiles(const string& pattern, vector<string> *result) {
  return GetMatchingFiles(pattern.c_str(), result);
}

// given a file name, create parent directories as needed
// (the file itself is not created automatically)
int CreateDirectoryIfNecessary(const char *file);
inline int CreateDirectoryIfNecessary(const string& file) {
  return CreateDirectoryIfNecessary(file.c_str());
}

// Backwards compatibility.
bool ReadFileToString(const string& file_name, string *content);

// This approach is slightly faster.
string ReadFileToString(const string& file_name, bool *success = nullptr);

// find memory usage in kilobytes
int GetMemoryUsage();

// count lines in file
int CountLines(ifstream& ifs);
int CountLines(const string& filename);

// returns the filenames in a directory (recursively if required). asserts if directory not found.
int GetFileNamesRecursively(const string& diranme, vector<string> *filenames,
                            const bool recurse = true);

// returns the filenames in a directory.
inline int GetFileNames(const string& dirname, vector<string> *filenames) {
  return GetFileNamesRecursively(dirname, filenames, false);
}

// Creates a random temporary directory using the pattern and registers a
// cleanup lambda function that removes the contents of the directory at program
// exit. Patterns must contain a section like XXXXX that will be randomized in a
// unique way across processes. For example this pattern:
// /tmp/tmp_dir.XXXXXX may produce a directory like /tmp/tmp_dir.q1hIg4
// Returns the generated (and created directory).
string MakeTempDir(const string& pattern);

// Example: JoinMultiple parts of a path. (vector<string> { "a", "b", "c" }) -> "a/b/c"
inline const string JoinPath(const vector<string>& parts) {
  return strutil::JoinString(parts, "/");
}

// Joins two strings using the given operator.
inline const string JoinPath(const string& left, const string& right) {
  vector<string> parts = {left, right};
  return JoinPath(parts);
}

// Returns the extension for the filename.
// NOTE: Includes the '.'
inline string GetExtension(const string& filename) {
  size_t pos = filename.find_last_of('.');
  return (pos != string::npos) ? filename.substr(pos) : "";
}

inline string ReplaceExtension(const string& filename, const string& extension) {
  string res = filename;
  size_t pos = filename.find_last_of('.');
  if (pos != string::npos) res.replace(pos, filename.size() - pos,  extension);
  else res += extension;
  return res;
}

inline bool HasExtension(const string& filename, const string& ext) {
  // Check if the size of file is less than the size of extension.
  if (filename.size() < ext.size()) return false;

  // Match the string.
  size_t expected_pos = filename.size() - ext.size();
  return (filename.find(ext, expected_pos) == expected_pos);
}

}  // namespace File

#endif  // _PUBLIC_UTIL_FILE_FILE_H_
