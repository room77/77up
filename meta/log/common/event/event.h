// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_EVENT_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_EVENT_H_

namespace logging {
namespace event {

// base event struct used as an interface between the components
// that do not necessarily know the type of data that is coming in
// all other event structs are derived from this one and represent
// the data coming from json request value field.
struct tEventBase {
  virtual ~tEventBase() {}
};

// Utility class that contains the data associated with an event.
template <typename EventData>
struct tEventWithData : public tEventBase {
  virtual ~tEventWithData() {}
  EventData data;
};

// Utility function to return the data for an event.
template<typename T>
const T& GetEventData(const tEventBase& event) {
  return dynamic_cast<const tEventWithData<T>&>(event).data;
}

// Utility function to return mutable data for an event.
template<typename T>
T* GetEventData(tEventBase* event) {
  return &(dynamic_cast<tEventWithData<T>*>(event)->data);
}

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_EVENT_H_
