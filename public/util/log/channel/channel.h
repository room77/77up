// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#ifndef _PUBLIC_UTIL_LOG_CHANNEL_CHANNEL_H_
#define _PUBLIC_UTIL_LOG_CHANNEL_CHANNEL_H_

#include <unordered_map>

#include "base/common.h"
#include "util/templates/container_util.h"

namespace meta {
namespace channel {

enum {
  // All the channels between WebBegin and WebEnd are treated as channels where
  // the complete site can be displayed.
  kChannelWebBegin = 0,
  kChannelDesktopBegin = 0,
  // Web/Desktop channel. By default, all unknown channels are treated as web.
  kChannelDesktopWeb,
  kChannelDesktopEnd = 49,

  // Tablet channels. These will also be considered desktop channels by default.
  kChannelTabletBegin = 50,
  kChannelTabletWeb,
  kChannelTabletAppIOS,
  kChannelTabletAppAndroid,
  kChannelTabletAppWindows,
  kChannelTabletAppOther,
  kChannelTabletEnd = 99,

  // Done with all web channels.
  kChannelWebEnd = 99,

  // All the channels between MobileBegin and MobileEnd are treated as mobile
  // channels.
  kChannelMobileBegin = 100,
  kChannelMobileWeb,
  kChannelMobileAppIOS,
  kChannelMobileAppAndroid,
  kChannelMobileAppWindows,
  kChannelMobileAppOther,
  kChannelMobileEnd = 149,
};
typedef unsigned int DeviceChannel;

// Returns the CGI parameter used to define channel.
inline string ChannelCGIParam() { return "channel"; }

// Utility function the get channel (as a string) from the arg_map.
inline string GetChannelString(const unordered_map<string, string>& arg_map) {
  return ::util::tl::FindWithDefault(arg_map, ChannelCGIParam(), "web");
}

// Given a string, returns the appropriate channel. On empty string
// returns the default channel
DeviceChannel GetChannel(const string& str);

// Utility function the get channel from the arg_map.
inline DeviceChannel GetChannel(const unordered_map<string, string>& arg_map) {
  return GetChannel(::util::tl::FindWithDefault(arg_map, ChannelCGIParam(), ""));
}

// @return the default channel. useful for backend methods that need channels
inline DeviceChannel DefaultChannel() { return GetChannel(""); }

// Returns true if channel is web. These are the channels where our full site
// can be displayed by default.
inline bool IsWeb(DeviceChannel channel) {
  return kChannelWebBegin < channel && channel < kChannelWebEnd;
}

// Returns true for desktop.
inline bool IsDesktop(DeviceChannel channel) {
  return kChannelDesktopBegin < channel && channel < kChannelDesktopEnd;
}

// Returns true for desktop web.
inline bool IsDesktopWeb(DeviceChannel channel) {
  return channel == kChannelDesktopWeb;
}

// Returns true for tablet. Can be either web or apps.
inline bool IsTablet(DeviceChannel channel) {
  return kChannelTabletBegin < channel && channel < kChannelTabletEnd;
}

// Returns true for desktop web.
inline bool IsTabletWeb(DeviceChannel channel) {
  return channel == kChannelTabletWeb;
}

// Returns true if channel is tablet app.
inline bool IsTabletApp(DeviceChannel channel) {
  return kChannelTabletAppIOS <= channel && channel <= kChannelTabletAppOther;
}

// Returns true if channel is mobile.
inline bool IsMobile(DeviceChannel channel) {
  return kChannelMobileBegin < channel && channel < kChannelMobileEnd;
}

// Returns true for mobile web.
inline bool IsMobileWeb(DeviceChannel channel) {
  return channel == kChannelMobileWeb;
}

// Returns true if channel is Mobile App.
inline bool IsMobileApp(DeviceChannel channel) {
  return kChannelMobileAppIOS <= channel && channel <= kChannelMobileAppOther;
}

}  // namespace channel
}  // namespace meta

#endif  // _PUBLIC_UTIL_LOG_CHANNEL_CHANNEL_H_
