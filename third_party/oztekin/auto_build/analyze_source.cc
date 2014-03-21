// Copyright 2013 B. Uygar Oztekin

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>

using namespace std;

// Limit analysis to top max_lines of each file for efficiency.
constexpr int max_lines = 500;

// Header and source extensions we will consider.
set<string> hdr_extensions = { "h", "hh", "hpp", "H"};
set<string> src_extensions = { "c", "cc", "cpp", "C" };

// If the system header on the left is included, assume we need to link in the
// library on the right.
map<string, vector<string>> header_to_lib = {
  { "aspell.h", {"aspell"}},
  { "future", {"pthread"} },
  { "mutex", {"pthread"} },
  { "thread",{"pthread"} },
  { "dlfcn.h", {"dl"} },
  { "zlib.h", {"z"} },
  { "gmock/gmock.h", {"gmock"} },
  { "gtest/gtest.h", {"gtest"} },
  { "leveldb/db.h", {"leveldb"} },
  { "leveldb/env.h", {"leveldb"} },
  { "leveldb/table.h", {"leveldb"} },
  { "leveldb/table_builder.h", {"leveldb"} },
  { "GeoIP.h", {"GeoIP"} },
  { "openssl/ssl.h", {"ssl", "crypto"} },
  { "google/heap-profiler.h", {"tcmalloc"} },
  { "unicode/translit.h", {"icuio", "icuuc", "icui18n"}},
};

// Similar in spirit to dirname shell command.
string Dirname(const string& path) {
  auto slash = path.find_last_of("/");
  return slash != string::npos ? path.substr(0, slash+1) : "";
}

// Returns true if file exists.
inline bool Exists(const string& filename) {
  return !ifstream(filename).fail();
}

// Returns the filename extension if exists.
inline string Extension(const string& path) {
  auto dot = path.find_last_of('.');
  if (dot == string::npos) return "";
  auto slash = path.find_last_of('/');
  return (slash == string::npos || slash < dot) ? path.substr(dot+1) : "";
}

// Remove the filename extension from path.
inline string StripExtension(const string& path) {
  auto dot = path.find_last_of('.');
  if (dot == string::npos) return path;
  auto slash = path.find_last_of('/');
  return (slash == string::npos || slash < dot) ? path.substr(0, dot) : path;
}

string NormalizePath(string path) {
  // Strip leading ./ if any.
  if (path.size() >= 2 && path[0] == '.' && path[1] == '/') path.erase(0, 2);

  // remove /X/../ occurences.
  for (auto pos = path.find("/../"); pos != string::npos && pos; pos = path.find("/../")) {
    auto prev_slash = path.find_last_of('/', pos-1);
    if (prev_slash != string::npos) path.erase(prev_slash + 1, pos - prev_slash + 3);
  }
  return path;
}

// Get a base directory and relative to dir or relative to . filename.
// - If file exists relative to directory, favor that one.
// - If not, check relative to `pwd`. Use it if found.
// Additionally, if file matches a header pattern, check to see if a
// corresponding C++ source can be found. If found, return that as well.
vector<string> Expand(const string& dirname, const string& filename) {
  vector<string> files;
  if (Exists(dirname + filename)) files.push_back(NormalizePath(dirname + filename));
  else if (Exists(filename))      files.push_back(NormalizePath(filename));
  if (!files.empty()) {
    string ext = Extension(files[0]);
    if (hdr_extensions.find(ext) != hdr_extensions.end()) {
      string base = StripExtension(files[0]);
      for (auto& s_pat : src_extensions) {
        if (Exists(base + "." + s_pat)) {
          files.push_back(NormalizePath(base + "." + s_pat));
          break;
        }
      }
    }
  }
  return files;
}

bool ProcessFile(string path, map<string, int>& srcs, map<string, int>& libs) {
  path = NormalizePath(path);
  string dirname = Dirname(path);
  ifstream file(path.c_str());
  if (file.fail()) return false;
  ++srcs[path];
  for (int i = 0; i < max_lines; ++i) {
    string line;
    getline(file, line);
    if (file.fail()) break;

    // If we have a system header, see if we need to add any libraries.
    int pos = line.find("#include <");
    if (pos != string::npos) {
      int begin = pos + 10;
      int end = line.find(">", begin);
      if (end != string::npos) {
        string token = line.substr(begin, end-begin);
        auto it = header_to_lib.find(token);
        if (it != header_to_lib.end())
          for (string& lib: it->second) ++libs[lib];
      }
    }

    // If we have a user header, expand and recursively process it.
    pos = line.find("#include \"");
    if (pos == string::npos) pos = line.find("@include \"");
    if (pos != string::npos) {
      int begin = pos + 10;
      int end = line.find("\"", begin);
      if (end != string::npos) {
        auto files = Expand(dirname, line.substr(begin, end-begin));
        for (auto& f : files) {
          if (!srcs[f]++) ProcessFile(f, srcs, libs);
        }
      }
    }
  }
  return true;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    cout << "Usage: " << argv[0] << " output_dir file1 [ file2 | ...]" << endl;
    cout << "    Analyzes source files and outputs .dep and .l files." << endl;
    return 0;
  }
  string out_dir = argv[1];
  if (!out_dir.empty() && *out_dir.rbegin() != '/') out_dir += '/';
  for (int i = 2; i < argc; ++i) {
    map<string, int> srcs, libs;
    ProcessFile(argv[i], srcs, libs);
    system((string("mkdir -p '") + out_dir + "'").c_str());
    string basename = StripExtension(argv[i]);
    {
      string filename = out_dir + basename + ".dep";
      system((string("mkdir -p '") + Dirname(filename) + "'").c_str());
      ofstream dep(filename.c_str());
      if (dep.fail()) {
        cerr << "Error opening file: '" << filename << "'" << endl;
        return 1;
      }
      dep << out_dir + basename << ":";
      for (auto& s : srcs) {
        string ext = Extension(s.first);
        if (src_extensions.find(ext) != src_extensions.end())
          dep << " " << out_dir << StripExtension(s.first) << ".o";
      }
      dep << endl;
    }
    {
      string filename = out_dir + "/" + basename + ".l";
      system((string("mkdir -p '") + Dirname(filename) + "'").c_str());
      ofstream link(filename.c_str());
      if (link.fail()) {
        cerr << "Error opening file: '" << filename << "'" << endl;
        return 1;
      }
      for (auto& l : libs) link << "-l" << l.first << " ";
      link << endl;
    }
  }
}
