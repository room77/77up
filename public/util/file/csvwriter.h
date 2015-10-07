#ifndef _PUBLIC_UTIL_FILE_CSVWRITER_H_
#define _PUBLIC_UTIL_FILE_CSVWRITER_H_

#include "base/common.h"

namespace CSV {

// This is used when the iterator simply points to Data (e.g. vector).
template<class Data, class Iterator>
struct DereferenceIterator {
  const Data& operator() (Iterator& iter) const {
    return *iter;
  }
};

// This is used when the iterator stores pair<?, Data> instead of Data (e.g.
// map).
template<class Data, class Iterator>
struct PairDereferenceIterator {
  const Data& operator() (Iterator& iter) const {
    return iter->second;
  }
};

// This is used when the iterator stores Data* instead of Data (e.g.
// Collection).
template<class Data, class Iterator>
struct DoubleDereferenceIterator {
  const Data& operator() (Iterator& iter) const {
    return *(*iter);
  }
};

template<class Data, class Iterator,
    class IterToData=DereferenceIterator<Data, Iterator> >
class CSVWriter {
 public:
  CSVWriter(const string& out_file, char delim = ',')
      : out_file_(out_file),
        delim_(delim) {}

  ~CSVWriter() {}

  bool Write(Iterator begin, Iterator end) {
    static const string kUTF8BOM = "\xef\xbb\xbf";

    VLOG(2) << "Writing " << out_file_;
    ofstream f(out_file_.c_str());
    if (!(f.good())) {
      LOG(INFO) << "Could not write to location: " << out_file_;
      return false;
    }

    // Write the UTF8 BOM.
    f << kUTF8BOM;

    // Write Header.
    Data dummy;
    f << dummy.ToCSV(delim_, true) << "\n";
    IterToData iter_to_data;

    for (Iterator iter = begin; iter != end; ++iter) {
      const Data& data = iter_to_data(iter);
      f << data.ToCSV(delim_, false) << "\n";
    }
    return true;
  }

 private:
  const string out_file_;
  const char delim_;
};

}  // namespace CSV

#endif  // _PUBLIC_UTIL_FILE_CSVWRITER_H_
