#ifndef _PUBLIC_UTIL_SERIAL_SERIALIZER_CSV_H_
#define _PUBLIC_UTIL_SERIAL_SERIALIZER_CSV_H_

// TODO(pramodg): Fix this file.

#include "base/defs.h"

//--------------------------------------------------------------------------
// CSV generation and parsing
//--------------------------------------------------------------------------

class SerializeCSV {
 public:
  SerializeCSV(char delimitor) : delimitor_(delimitor), count_(0) {}

  string str() const { return ss_.str(); }

  // define operator | for all data types
  template <class T>
  SerializeCSV& operator|(const T& v) {
    if (count_ > 0)
      ss_ << delimitor_;
    Print(v);
    count_++;
    return *this;
  }

  // print a string after escaping it
  void Print(const string& v) { ss_ << strutil::EscapeString_C(v); }
  void Print(string& v) { ss_ << strutil::EscapeString_C(v); }

/*
  template<class T, class Key, int Id>
  void Print(const ::util::unique<T, Key, Id>& v) {
    Print(v.get());
  }
*/

  // print all other data types directly
  template <class T>
  void Print(const T& v) { ss_ << v; }

  // generate CSV header line with field names
  static string MakeCSVHeader(const string& varlist, char delimitor) {
    vector<string> names;
    strutil::BreakUpString(varlist, '|', &names);
    SerializeCSV s(delimitor);
    for (int i = 0; i < names.size(); i++) {
      s | (names[i]);
    }
    return s.str();
  }

 private:
  stringstream ss_;
  char delimitor_;
  int count_;
};

class DeserializeCSV {
 public:
  DeserializeCSV(const string& st, char delimitor)
    : st_(st), delimitor_(delimitor) {
    Init(st_.c_str(), st_.size());
  }
  DeserializeCSV(const char *str, int len, char delimitor)
    : delimitor_(delimitor) {
    Init(str, len);
  };
  ~DeserializeCSV() {};

  inline void Init(const char *str, int len) {
    str_ = str;
    ptr_ = str_;  // beginning of string
    end_ = str_ + len;  // end of string (\0)
    field_count_ = 0;
    ok_ = true;
  }
  inline bool ok() const { return ok_; };
  // inline bool reached_end() const { return (ptr_ == end_); };
  inline int bytes_parsed() const { return (ptr_ - str_); };
  inline string get_error_msg() const { return error_msg_.str(); };

  inline void End() {
    if (ok_) {
      ptr_ = strutil::SkipSpaces(ptr_);
      if (*ptr_ != '\0' && *ptr_ != '#')
        Error("Extra characters after the end", ptr_ - str_);
    }
  }

  // define operator | to be >> for this class
  template <class T>
    inline DeserializeCSV& operator|(T& v) {
    if (ok_) {
      // expect a delimitor if this is not the first field
      if (field_count_ > 0) {
        if (*ptr_ == delimitor_)
          ptr_++;
        else
          Error("syntax error", ptr_ - str_);
      }

      if (ok_) {
        const char *s = strutil::SkipSpaces(ptr_);
        string field;
        if ((*s) == '"') {
          // unescape the field first
          int num_bytes = strutil::UnescapeString_C(s, &field);
          ptr_ = s + num_bytes;
        }
        else {
          // skip until the next delimitor or '\0'
          ptr_ = s;
          while ((*ptr_) != '\0' && (*ptr_) != delimitor_)
            ptr_++;
          field = string(s, ptr_ - s);
        }
        const char *endptr;
        Parse(field.c_str(), &endptr, v);
        if (ok_) {
          endptr = strutil::SkipSpaces(endptr);
          if ((*endptr) != '\0' && (*endptr) != '#')
            Error("Extra characters in field",
                  s - str_ + (endptr - field.c_str()));
        }
      }

      if (ok_) {
        // skip spaces other than delimitor
        while ((*ptr_) <= ' ' && (*ptr_) != '\0' && (*ptr_) != delimitor_)
          ptr_++;
      }
    }
    field_count_++;
    return *this;
  }

  inline void Parse(const char *start, const char **endptr, int& v) {
    char *e;
    v = strtol(start, &e, 10);
    *endptr = e;
    if (start == e)
      v = 0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr, unsigned int& v) {
    char *e;
    v = strtoul(start, &e, 10);
    *endptr = e;
    if (start == e)
      v = 0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr, short& v) {
    char *e;
    v = strtol(start, &e, 10);
    *endptr = e;
    if (start == e)
      v = 0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr,
                    unsigned short& v) {
    char *e;
    v = strtoul(start, &e, 10);
    *endptr = e;
    if (start == e)
      v = 0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr, char& v) {
    char *e;
    v = strtol(start, &e, 10);
    *endptr = e;
    if (start == e)
      v = 0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr, unsigned char& v) {
    char *e;
    v = strtoul(start, &e, 10);
    *endptr = e;
    if (start == e)
      v = 0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr, long long& v) {
    char *e;
    v = strtoll(start, &e, 10);
    *endptr = e;
    if (start == e)
      v = 0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr,
                    unsigned long long& v) {
    char *e;
    v = strtoull(start, &e, 10);
    *endptr = e;
    if (start == e)
      v = 0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr, long int& v) {
    // assume 64-bit platform - long int is same as long long
    char *e;
    v = strtoll(start, &e, 10);
    *endptr = e;
    if (start == e)
      v = 0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr,
                    unsigned long int& v) {
    // assume 64-bit platform - long int is same as long long
    char *e;
    v = strtoull(start, &e, 10);
    *endptr = e;
    if (start == e)
      v = 0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr, float& v) {
    char *e;
    v = strtof(start, &e);
    *endptr = e;
    if (start == e)
      v = 0.0;  // set to 0 if input is empty
  }
  inline void Parse(const char *start, const char **endptr, double& v) {
    char *e;
    v = strtod(start, &e);
    *endptr = e;
    if (start == e)
      v = 0.0;  // set to 0 if input is empty
  }

  void Parse(const char *start, const char **endptr, bool& v) {
    if (!strncasecmp(start, "true", 4)) {
      v = true;
      *endptr = start + 4;
    }
    else if (!strncasecmp(start, "false", 5)) {
      v = false;
      *endptr = start + 5;
    }
    else {
      Error("true or false expected", start - str_);
      *endptr = start;
    }
  }

  void Parse(const char *start, const char **endptr, string& v) {
    v = string(start);
    *endptr = start + v.size();
  }

  /* This is not supported. This will be deleted in subsequest cl.
  void Parse(const char *start, const char **endptr, const char *& v) {
    v = gStringStorage.Store(start);
    *endptr = start + strlen(start);
  }
  */

/*
  template<class T, class Key, int Id>
  void Parse(const char *start, const char **endptr,
             ::util::unique<T, Key, Id>& v) {
    T t;
    Parse(start, endptr, t);
    v = t;
  }
*/

  template<typename T>
  void Parse(const char *start, const char **endptr, T& v) {
    const string str = string(start);
    stringstream ss(str);
    ss >> v;
    if (ss.tellg() == -1)
      *endptr = start + str.size();
    else
      *endptr = start + ss.tellg();
  }

  // check if header information in the first line of CSV file matches
  // this struct's field names
  bool CheckHeaderLine(const string& varlist) {
    vector<string> names;
    strutil::BreakUpString(varlist, '|', &names);
    for (int i = 0; ok_ && i < names.size(); i++) {
      string v;
      operator|(v);
      if (ok_) {
        if (!strutil::SameName(v, names[i])) {
          // header column name mismatch
          if (names[i] != "gDummy")
            Error(string("Field name does not match CSV header: ")
                  + v + " vs. " + names[i],
                  ptr_ - str_);
        }
      }
    }
    End();
    return ok_;
  }

 private:
  bool ok_;  // false if an error has occurred
  const string st_;
  char delimitor_;
  const char *str_;  // beginning of string to be parsed
  const char *ptr_;  // points to the string position being parsed
  const char *end_;  // points to the end of the string
  int field_count_;
  stringstream error_msg_;

  void Error(const string& error_msg, int error_loc) {
    ok_ = false;
    error_msg_ << "Error at char " << (error_loc + 1) << ": " << error_msg;
  }
};

#endif  // _PUBLIC_UTIL_SERIAL_SERIALIZER_CSV_H_

