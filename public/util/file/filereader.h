#ifndef _PUBLIC_UTIL_FILE_FILEREADER_H_
#define _PUBLIC_UTIL_FILE_FILEREADER_H_

#include <fstream>

#include "util/file/monitor_file_access.h"
#include "util/memory/collection.h"
#include "util/serial/serializer.h"

// utilities to read/print data files

namespace FileReader {

template<class DataType>
function<bool(istream&, DataType *)> BinaryParser() {
  return [](istream& str, DataType *buf) {
    return serial::Serializer::FromBinary<DataType>(str, buf);
  };
}

template<class DataType>
inline function<bool(istream&, DataType *)> JSONParser() {
  return [](istream& str, DataType *buf) {
    return serial::Serializer::FromJSON(str, buf);
  };
}

// JSON requires quotes around strings
// use this for parsing lines as strings
inline function<bool(istream&, string *)> StringLineParser() {
  return [](istream& str, string *buf) {
    getline(str, *buf);
    return !str.fail();
  };
}

// Extract entries of type DataType from file and call the function with each.
// Assert can be set to false, in which case function returns -1 on failure.
// template parameter specialization did not work, so parameters are used
// instead. Direct questions to Pramod
template<class DataType, class Callback,
         class Parser=function<bool(istream&, DataType*)> >
int ProcessAll(const string& input_file, Callback callback, bool assert=true,
    Parser parse_func=BinaryParser<DataType>()) {
  VLOG(2) << "Reading " << input_file;
  ifstream f(input_file.c_str(), ios::in);
  if (assert) ASSERT(f.good()) << "File not found: " << input_file;
  else if (!f.good()) return -1;

  bool status = 0;
  int count = 0;
  while (true) {
    DataType buf;
    status = parse_func(f, &buf);
    if (!status) break;

    ++count;
    // Allow move constructor in callbacks. buf cannot be used after this.
    callback(std::move(buf));

    // If we are in record file access mode. Return fast after reading the
    // first entry.
    if (gFlag_record_file_access) {
      status = 0;
      break;
    }
  }
  if (assert) ASSERT(status == 0) << "Error reading file " << input_file;
  f.close();
  return status == 0 ? count : -1;
}

// Read the data as a standard STL container. This should work with most
// non-associative containers (e.g. vector, dequeue, list, set, etc).
// If you don't need Collection or UniqueCollection, this function may be the
// one you want.
template<class Container, class DataType=typename Container::value_type>
Container ReadAllAs(const string& input_file,
    function<bool(istream&, DataType*)> parse_func = BinaryParser<DataType>()) {
  Container ret;
  ProcessAll<DataType>(input_file,
    [&ret](DataType&& d){ ret.insert(ret.end(), d); }, true, parse_func);
  return ret;
}

// fill a container (UniqueCollection or Collection) with data from
// a binary file (RPC-serialized format)
template<class Collection, class DataType =
    typename std::remove_pointer<typename Collection::value_type>::type>
int ReadAll(const string& input_file, Collection *output,
    function<bool(istream&, DataType*)> parse_func =
        BinaryParser<DataType>()) {
  return ProcessAll<DataType>(input_file,
      [output](DataType&& d){ output->Store(d); }, true, parse_func);
}

// print data file content to screen (for debugging)
template<class DataType, class Parser=function<bool(istream&, DataType *)> >
int PrintAll(const string& input_file, Parser parse_func=BinaryParser<DataType>()) {
  return ProcessAll<DataType>(input_file,
                              [](const DataType& buf)
                              { cout << serial::Serializer::ToJSON(buf) << endl; },
                              false, parse_func);
}

}

#endif  // _PUBLIC_UTIL_FILE_FILEREADER_H_
