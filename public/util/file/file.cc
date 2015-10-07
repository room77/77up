// Copyright 2007-2013 Optrip, Room77
// Author: Calvin Yang, Pramod Gupta, Uygar Oztekin

#include "util/file/file.h"

#include <dirent.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <algorithm>
#include <queue>

#include "base/args/args.h"
#include "util/init/init.h"

namespace file {

// given a file pattern, return all matching files
int GetMatchingFiles(const char *pattern,
                     vector<string> *result) {
  VLOG(3) << "File pattern: " << pattern;

  result->clear();

  glob_t globbuf;
  globbuf.gl_offs = 0;
  int ret = glob(pattern, 0, NULL, &globbuf);
  if (ret != 0) {
    VLOG(3) << "Error retrieving matching files.";
    return 0;
  }

  size_t num_files = globbuf.gl_pathc;
  char **filename = globbuf.gl_pathv;

  result->reserve(num_files);
  for (int i = 0; i < num_files; i++)
    result->push_back(filename[i]);
  globfree(&globbuf);

  VLOG(3) << num_files << " files found.";
  return num_files;
}

// given a file name, create parent directories as needed
// (the file itself is not created automatically)
int CreateDirectoryIfNecessary(const char *file) {
  int count = 0;
  const char *s = file;
  while ((*s) != '\0') {
    s++;
    if ((*s) == '/') {
      // create this directory, if necessary
      string dir(file, s - file);

      // permission: read/write/execute by everybody (i.e., let the process'
      //   file create mask decide the mode)
      int ret = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
      if (ret == 0) {
        VLOG(3) << "created directory " << dir;
        count++;
      }
      else {
        if (errno != EEXIST)
          ASSERT(false) << "Unable to create directory " << dir << ": "
                        << strutil::LastSystemError();
      }
    }
  }
  return count;
}

bool ReadFileToString(const string& file_name, string *content) {
  *content = "";
  if (!IsFile(file_name)) return false;
  ifstream in(file_name);
  ostringstream out;
  copy(istreambuf_iterator<char>(in), istreambuf_iterator<char>(),
       ostreambuf_iterator<char>(out));
  *content = out.str();
  return in.good();
}

string ReadFileToString(const string& file_name, bool* success) {
  if (success != nullptr) *success = false;
  if (!IsFile(file_name)) return "";
  ifstream in(file_name);
  ostringstream out;
  copy(istreambuf_iterator<char>(in), istreambuf_iterator<char>(),
       ostreambuf_iterator<char>(out));
  if (success != nullptr) *success = in.good();
  return out.str();
}

int GetMemoryUsage() {
  stringstream ss;
  ss << "/proc/" << getpid() << "/status";
  string proc_status;
  file::ReadFileToString(ss.str(), &proc_status);
  // LOG(INFO) << "Proc status: " << proc_status << "***";
  const char *s = proc_status.c_str();
  const char *vm = strstr(s, "VmSize:");
  if (vm) {
    const char *num = strutil::SkipSpaces(vm + 7);
    return atoi(num);
  }
  return -1;
}

int CountLines(ifstream& ifs) {
  streampos pos = ifs.tellg();
  if (!ifs.good()) return -1;
  int lines = 1;
  while (ifs.good()) {
    lines += (ifs.get() == '\n');
  }
  ifs.seekg(pos, ios::beg);
  ifs.clear();
  return lines;
}

int CountLines(const string& filename) {
  ifstream ifs(filename);
  if (!ifs.good()) return -1;
  int lines = 1;
  while (ifs.good()) {
    lines += (ifs.get() == '\n');
  }
  ifs.close();
  return lines;
}

int GetFileNamesRecursively(const string& dirname, vector<string> *filenames, const bool recurse) {
  std::queue<string> dirs;
  dirs.push(dirname);
  // recurse through directories in the queue while it's not empty
  while (!dirs.empty()) {
    const string current_dir = dirs.front();
    dirs.pop();
    DIR *dp = opendir(current_dir.c_str());
    ASSERT_NOTNULL(dp) << "Error opening " << current_dir;

    struct dirent *dirp;
    while ((dirp = readdir(dp)) != nullptr) {
      // take action dpending on the type of item in the directory
      string name(dirp->d_name);
      if (name == "." || name == "..") continue;
      string filename = JoinPath(current_dir, name);
      if (IsDirectory(filename)) {
        if (recurse) dirs.push(filename);
        continue;
      }
      filenames->push_back(filename);
    }
    closedir(dp);
  }

  // Sort files in alphabetical order.
  sort(filenames->begin(), filenames->end());

  return filenames->size();
}

string MakeTempDir(const string& pattern) {
  string tmpdir = pattern;
  ASSERT(mkdtemp(&tmpdir[0]));
  init::ExitAdd("clean_tmp_dir", FILE_AND_LINE, [tmpdir]{
    int status = system(("rm -rf " + tmpdir).c_str());
    ASSERT_DEV_LE(status, 0);
  });
  return tmpdir;
}

}  // namespace File
