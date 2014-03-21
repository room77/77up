// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "meta/log/offline/reader/element_readers/element_reader_lm.h"

#include "meta/log/common/event/parser/event_parser.h"
#include "meta/log/common/manager/log_manager.h"
#include "meta/log/offline/user_log_element.h"
#include "util/serial/serializer.h"

namespace logging {
namespace reader {

void ReadLogElemFwdToLogManagerBase::ForwardElementToLogManager(
    const shared_ptr<tLogElement>& elem, const string& parser_type) const {
  // Forward this element to the log manager.
  // TODO(otasevic): consider adding a flag "filtering"(similar to async). In case the flag is false
  // call the methods without element_info
  if (async) LogManager::Instance().ProcessLogElementAsync(elem, parser_type);
  else LogManager::Instance().ProcessLogElement(elem, parser_type);
}

void ReadLogElemFwdToLogManagerBase::Wait() const {
  LogManager::Instance().Wait();
}

int ReadJSONLogElemFwdToLogManager::Read(const string& element, const string& filename) const {
  tUserLogReadBackElement* user_elem = new tUserLogReadBackElement;
  shared_ptr<tLogElement> elem(user_elem);
  ASSERT(user_elem != nullptr);
  // If we failed to parse the JSON stream
  if (!user_elem->FromJSON(element)) {
    LOG(ERROR) << "Failed to parse: " << element;
    return 0;
  }
  // note the filename from which the element was read
  user_elem->element_add_info.read_filename = filename;

  ForwardElementToLogManager(elem, event::EventParserKey::JSONKey());
  return element.size();
}

int ReadBinaryLogElemFwdToLogManager::Read(const string& element, const string& filename) const {
  // Check if the binary stream is complete. If not, ask for more data.
  if (!serial::Serializer::BinaryStreamHasEnoughDataToParsePrependedSize<unsigned int>(element))
    return -1;

  tUserLogReadBackElement* user_elem = new tUserLogReadBackElement;
  shared_ptr<tLogElement> elem(user_elem);
  // If we failed to parse the JSON stream
  if (!serial::Serializer::FromBinaryPrependedSize<unsigned int>(element, user_elem)) {
    LOG(ERROR) << "Failed to parse: " << element;
    return 0;
  }

  ForwardElementToLogManager(elem, event::EventParserKey::BinaryKey());

  // Here is the catch. We always expect each element to finish with a new line which is
  // automatically absorbed. If this is not true, we have no good way of returning how much of the
  // input stream we did not consume. Currently this is how logging is supposed to happen. However,
  // if this changes in future, we may have to take another look at this.
  // TODO(pramodg): We may be able to do this with appropriate return values from
  // BinaryStreamHasEnoughDataToParsePrependedSize.
  return 1;
}

}  // namespace reader
}  // namespace logging
