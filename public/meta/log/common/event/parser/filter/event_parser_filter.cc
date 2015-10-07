// Copyright 2013 Room77, Inc.
// Author: otasevic@room77.com (Nikola Otasevic)


#include "base/defs.h"
#include "util/init/init.h"
#include "util/string/strutil.h"
#include "meta/log/common/event/datatypes/event_filter.h"
#include "meta/log/common/event/parser/event_parser.h"
#include "util/serial/serializer.h"

namespace logging {
namespace event {

// Parser for tEventFilterSelectedItem.
class EventJSONParserForFilterSelectedItem
    : public EventJSONParserForType<tEventFilterSelectedItem> {
 public:
  virtual ~EventJSONParserForFilterSelectedItem() {}

 protected:
  // Parses the object from the corresponding string.
  virtual bool ParseFromString(const string& str, tEventFilterSelectedItem* event_data) const {
    if (str.empty()) return false;

    string to_parse;
    if (str[0] == '"') serial::Serializer::FromJSON(str, &to_parse);
    else to_parse = str;

    vector<string> parts;
    if (strutil::RSplitStringNParts(to_parse, ":", &parts, 1) != 2) return false;

    // Set the name.
    event_data->name = parts[0];

    // Set the state.
    serial::Serializer::FromJSON(parts[1], &(event_data->state));
    return true;
  }
};

// Parser for [Filter, Neighborhood].
class EventJSONParserForFilterNeighborhood
    : public EventJSONParserForFilterSelectedItem {
  typedef EventJSONParserForFilterSelectedItem super;

 public:
  virtual ~EventJSONParserForFilterNeighborhood() {}

 protected:
  // Parses the object from the corresponding string.
  virtual bool ParseFromString(const string& str, tEventFilterSelectedItem* event_data) const {
    if (!super::ParseFromString(str, event_data)) return false;

    // Fix the name of the neighborhood.
    size_t pos = event_data->name.find('(');
    if (pos != string::npos) event_data->name.resize(pos -1);  // Also remove the last space.

    return true;
  }
};

// Parser for tEventFilterSpecialRates.
class EventJSONParserForFilterSpecialRates
    : public EventJSONParserForType<tEventFilterSpecialRates> {
 public:
  virtual ~EventJSONParserForFilterSpecialRates() {}

 protected:
  // Parses the object from the corresponding string.
  virtual bool ParseFromString(const string& str, tEventFilterSpecialRates* event_data) const {
    if (str.empty()) return false;

    // Check if this can be parsed as simple JSON.
    if (str[0] == '{') return ::serial::Serializer::FromJSON(str, event_data);

    string to_parse;
    if (str[0] == '"') serial::Serializer::FromJSON(str, &to_parse);
    else to_parse = str;

    vector<string> parts;
    if (strutil::RSplitStringNParts(to_parse, "_", &parts, 3) != 4) return false;

    event_data->aaa = strutil::ParseInt(parts[0]);
    event_data->senior = strutil::ParseInt(parts[1]);
    event_data->govt = strutil::ParseInt(parts[2]);
    event_data->mil = strutil::ParseInt(parts[3]);

    return true;
  }
};

// Parser for tEventFilterPrice.
class EventJSONParserForFilterPrice : public EventJSONParserForType<tEventFilterPrice> {
 public:
  virtual ~EventJSONParserForFilterPrice() {}

 protected:
  // Parses the object from the corresponding string.
  virtual bool ParseFromString(const string& str, tEventFilterPrice* event_data) const {
    // Consider this as someone wanted to select cheap.
    if (str.empty()) return true;

    // Check if this can be parsed as simple JSON.
    if (str[0] == '{') return ::serial::Serializer::FromJSON(str, event_data);
    else if (str[0] == '"') {
      string temp;
      return ::serial::Serializer::FromJSON(str, &temp) &&
          ::serial::Serializer::FromJSON(temp, event_data);
    }

    vector<string> parts;
    if (strutil::RSplitStringNParts(str, ":", &parts, 1) != 2) return false;

    event_data->min = -1;  // Set this to -1 to specify only the max value is valid.
    event_data->max = strutil::ParseInt(parts[0]);
    serial::Serializer::FromJSON(parts[1], &(event_data->state));
    return true;
  }
};

INIT_ADD_REQUIRED("filter_parsers", []() {
  EventParserInterface::bind(EventParserJSONKey("Filter", "Star Rating"),
      [] { return new EventJSONParserForFilterSelectedItem(); });
  EventParserInterface::bind(EventParserJSONKey("Filter", "Reviews"),
      [] { return new EventJSONParserForFilterSelectedItem(); });
  EventParserInterface::bind(EventParserJSONKey("Filter", "Amenities"),
      [] { return new EventJSONParserForFilterSelectedItem(); });
  EventParserInterface::bind(EventParserJSONKey("Filter", "Value"),
      [] { return new EventJSONParserForFilterSelectedItem(); });
  EventParserInterface::bind(EventParserJSONKey("Filter", "Views"),
      [] { return new EventJSONParserForFilterSelectedItem(); });

  EventParserInterface::bind(EventParserJSONKey("Filter", "Neighborhood"),
      [] { return new EventJSONParserForFilterNeighborhood(); });

  EventParserInterface::bind(EventParserJSONKey("Filter", "Special Rates"),
      [] { return new EventJSONParserForFilterSpecialRates(); });

  EventParserInterface::bind(EventParserJSONKey("Filter", "Price"),
      [] { return new EventJSONParserForFilterPrice(); });
});

}  // namespace event
}  // namespace logging

