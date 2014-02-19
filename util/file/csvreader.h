#ifndef _PUBLIC_UTIL_FILE_CSVREADER_H_
#define _PUBLIC_UTIL_FILE_CSVREADER_H_

#include <fstream>

#include "base/common.h"
#include "util/memory/collection.h"
#include "util/index/index.h"
#include "util/string/strutil.h"
#include "util/file/monitor_file_access.h"

namespace {

inline int CountOccurence(const string& str, char c) {
  int count = 0;
  for (int i = 0; i < str.size(); ++i)
    if (str[i] == c) ++count;
  return count;
}

inline string GetEntry(istream& f, bool multiple_line_ok) {
  string line;
  getline(f, line);
  while (multiple_line_ok && CountOccurence(line, '"') % 2) {
    string new_line;
    getline(f, new_line);
    if (f.fail()) break;
    line += new_line;
  }
  //replace(line.begin(), line.end(), '\r', ' ');
  return line;
}

}  // namespace

namespace CSV {

// This is the default inserter used to add to data to container
// (e.g. vector<Data>, Collection).
template<class Container, typename Data=typename Container::value_type >
struct DefaultContainerInserter {
  DefaultContainerInserter(Container* container) : container(container) {}

  void operator() (Data& data) const {
    container->insert(container->end(), data);
  }
  Container* container;
};

// This is the inserter used to add to data to container that holds a
// pointer to the data (e.g. vector<Data*>).
template<class Container, typename Data>
struct PointerContainerInserter {
  PointerContainerInserter(Container* container) : container(container) {}

  void operator() (Data& data) const {
    container->insert(container->end(),
                      typename Container::value_type(new Data(data)));
  }
  Container* container;
};

// This is a more generic reader as compared to the one above.
// Once all the clients have migrated to this, we can deprecate the function
// in namespace CSVReader.
template<class Container, typename Data=typename Container::value_type,
    typename Inserter=DefaultContainerInserter<Container, Data> >
class CSVReader {
 public:
  CSVReader(const string& in_file, char delim = ',')
      : in_file_(in_file),
        delim_(delim) {}

  ~CSVReader() {}

  bool Read(Container* output, bool multiple_line_ok = false) {
    static const string kUtf8Bom = "\xef\xbb\xbf";
    VLOG(2) << "Reading " << in_file_;
    ifstream f(in_file_.c_str());
    if (!(f.good())) {
      LOG(INFO) << "File not found: " << in_file_;
      return false;
    }

    string error_msg;
    string line;
    int line_num = 0;

    line = GetEntry(f, multiple_line_ok);
    if (line.find(kUtf8Bom) == 0) line = line.substr(3);
    if (!line.empty() || !f.eof()) {
      // Parse the first line as header.
      Data header;
      if (!(header.FromCSV(line, delim_, true, &error_msg))) {
        LOG(INFO) << "Error in file " << in_file_ << ", line 1 (CSV header): "
               << error_msg;
        f.close();
        return false;
      }
      line_num = 1;

      Inserter inserter(output);

      line = GetEntry(f, multiple_line_ok);
      while (!line.empty() || !f.eof()) {
        ++line_num;
        string trimmed = strutil::GetTrimmedString(line);
        if (!(trimmed.empty() || trimmed[0] == '#')) {
          Data obj;
          if (!(obj.FromCSV(line, delim_, false, &error_msg))) {
            LOG(INFO) << "Error in file " << in_file_ << ", line " << line_num
                   << ": " << error_msg;
            LOG(INFO) << line;
            f.close();
            return false;
          }
          // Add the object to collection.
          inserter(obj);
        }
        if (line_num % 100000 == 0)
          VLOG(2) << line_num << " lines read";
        line = GetEntry(f, multiple_line_ok);   // read another line
        // If we are in record file access mode, return fast after reading the
        // first entry.
        if (gFlag_record_file_access) break;
      }
    }
    f.close();
    VLOG(2) << "... " << line_num << " lines parsed.";
    return true;
  }

 private:
  const string in_file_;
  const char delim_;
};

}  // namespace CSV

namespace CSVReader {

// Fill a container (of type Collection) with data from a CSV file.
// Once all the functions directly start using the class above, this function
// can be deprecated.
template<class Container>
bool ReadFromCSV(const string& input_file, char delimitor,
                 Container *output, bool multiple_line_ok = false) {
  CSV::CSVReader<Container, typename Container::object_type> reader(input_file,
                                                                    delimitor);
  return reader.Read(output, multiple_line_ok);
}

}  // namespace CSVReader

#endif  // _PUBLIC_UTIL_FILE_CSVREADER_H_
