// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Interface for stopwords.

#ifndef _UTIL_I18N_STOPWORDS_STOPWORDS_MOCK_H_
#define _UTIL_I18N_STOPWORDS_STOPWORDS_MOCK_H_

#include "util/string/stopwords/stopwords.h"

#include "base/defs.h"
#include "util/factory/factory_extra.h"

namespace i18n {
namespace test {

class MockStopWords : public StopWords {
 public:
  // By default return true for these.
  virtual bool Configure(const string& opts) { return true; }
  virtual bool Initialize() {return true; };

  MOCK_CONST_METHOD1(IsStopWord, bool(const string& word));
};

inline void RegisterNewMockStopWords(const string& id, const string& params = "") {
  StopWords::bind(id, "", InitializeConfigureConstructor<MockStopWords, string>());
}

}  //  namespace test
}  // namespace i18n


#endif  // _UTIL_I18N_STOPWORDS_STOPWORDS_MOCK_H_
