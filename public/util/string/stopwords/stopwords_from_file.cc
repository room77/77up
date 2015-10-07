// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/string/stopwords/stopwords.h"

#include <unordered_set>

#include "base/args/args.h"
#include "util/factory/factory_extra.h"
#include "util/file/filereader.h"

FLAG_string(stopwords_dir, "static_data/push/auto/i18n/stopwords",
            "Default dir for abbreviations.");

FLAG_string(stopwords_small, "stopwords_small",
            "The name of the file that contains only small stopwords.");

FLAG_string(stopwords_all, "stopwords_all",
            "The name of the file that contains all stopwords.");

namespace i18n {

// Reads the stopwords from a file.
class StopWordsFromFile : public StopWords {
 public:
  virtual ~StopWordsFromFile() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) {
    file_ = opts;
    return file_.size();
  }

  // Initialize the class.
  virtual bool Initialize() {
    LOG(INFO) << "Initializing stopwords with file: " << file_;
    if (file_.empty()) {
        LOG(ERROR) << "Invalid file: " << file_;
        return false;
    }

    // Read the file to stopwords.
    stopwords_ = FileReader::ReadAllAs<unordered_set<string>>(file_,
        FileReader::StringLineParser());

    LOG(INFO) << "Initialized stopwords with file: " << file_ << ". Found "
              << stopwords_.size() << " stopwords.";
    return true;
  }

  // Returns true if the input string is a stop word.
  virtual bool IsStopWord(const string& word) const {
    return stopwords_.find(word) != stopwords_.end();
  }

 protected:
  // The file to read the stopwords from.
  string file_;

  // Set of stopwords.
  unordered_set<string> stopwords_;
};

// Reads the stopwords for a given language from the default fileset.
class StopWordsFromLanguage : public StopWordsFromFile {
  typedef StopWordsFromFile super;

 public:
  StopWordsFromLanguage() {
    // This call is not allowed.
    ASSERT(false);
  }

  explicit StopWordsFromLanguage(const string& file_name) : file_name_(file_name) {}

  virtual ~StopWordsFromLanguage() {}

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) {
    return super::Configure(gFlag_stopwords_dir + "/" + opts + "/" + file_name_);
  }

 protected:
  string file_name_;
};

static const string kEmptyString;
// Register different abbreviation types.
auto register_stopwords_small = StopWords::bind("stopwords_small", kEmptyString,
    InitializeConfigureWithConstructorParams<StopWordsFromLanguage, string, string>(kEmptyString,
        gFlag_stopwords_small));

auto register_stopwords_all = StopWords::bind("stopwords_all", kEmptyString,
    InitializeConfigureWithConstructorParams<StopWordsFromLanguage, string, string>(kEmptyString,
        gFlag_stopwords_all));

}  // namespace i18n
