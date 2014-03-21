// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// Interface for stopwords.

#ifndef _UTIL_I18N_STOPWORDS_STOPWORDS_H_
#define _UTIL_I18N_STOPWORDS_STOPWORDS_H_

#include "base/defs.h"
#include "base/logging.h"
#include "util/factory/factory.h"

namespace i18n {

// The basic stopwords interface.
class StopWords : public Factory<StopWords, string, string> {
 public:
  virtual ~StopWords() {}

  // Utility function to create the stopwords factory and pin it.
  static shared_proxy Create(const string& id, const string& params = "") {
    shared_proxy proxy = make_shared(id, params);
    ASSERT_NOTNULL(proxy);
    // We need to pin the proxy.
    pin(proxy);
    return proxy;
  }

  // Configuration parameters for the class.
  virtual bool Configure(const string& opts) { return true; }
  // Initialize the class.
  virtual bool Initialize() {return true; };

  // Returns true if the input string is a stop word.
  virtual bool IsStopWord(const string& word) const = 0;
};

}  // namespace i18n


#endif  // _UTIL_I18N_STOPWORDS_STOPWORDS_H_
