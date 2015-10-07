// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

// This file contains all the events associated with the "Filter" category.

#ifndef _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_FILTER_H_
#define _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_FILTER_H_

#include "base/defs.h"
#include "util/serial/serializer.h"

namespace logging {
namespace event {

////////////////////////////////////////////////////////////
// EVENT STRUCTS
////////////////////////////////////////////////////////////

// Action "Star Rating". e.g. "2-Star: false"
// Action "Reviews". e.g. "3-Review: false"
// Action "Amenities/Value/Views". e.g. "ADA Accessible: false"
// Action "Neighborhood". e.g. "Bel Air (1): true"
struct tEventFilterSelectedItem {
  // The name associated with the checkbox.
  string name;

  // The state of the selected item.
  // True if the item was selected, false if it was unselected.
  bool state = true;

  SERIALIZE(DEFAULT_CUSTOM / name*1 / state*2);
};

// Action "Distance". e.g. "10 mi" or "max mi"
typedef string tEventFilterDistance;

// Action "Special Rates". e.g. {"filter_name":"special rates","aaa":0,"senior":0,"govt":0,"mil":0}
// or ""0_0_0_0"
struct tEventFilterSpecialRates {
  // Set(=1) if AAA rates are enabled.
  bool aaa = 0;

  // Set(=1) if Senior rates are enabled.
  bool senior = 0;

  // Set(=1) if Govt. rates are enabled.
  bool govt = 0;

  // Set(=1) if Military rates are enabled.
  bool mil = 0;

  SERIALIZE(DEFAULT_CUSTOM / aaa*1 / senior*2 / govt*3 / mil*4);
};

// Action "Price". e.g.
// 1. ""(treated as cheap value),
// 2. "{\"min\":31,\"max\":88}"
// 3. "140: false" : Treated as max value.
struct tEventFilterPrice {
  // The min value of the slider.
  double min = 0;

  // The max value of the slider.
  double max = 0;

  // Special case when only the max value is specified, but the value is either set or reset.
  bool state = false;

  SERIALIZE(DEFAULT_CUSTOM / min*1 / max*2 / state*3);
};

}  // namespace event
}  // namespace logging


#endif  // _PUBLIC_META_LOG_COMMON_EVENT_DATATYPES_EVENT_FILTER_H_
