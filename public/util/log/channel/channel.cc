// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

//==========================================================================
// NOTE If you make changes to the strings here, don't forget to make the
// same changes to:
// Map in php/util/channel.inc.
//==========================================================================

#include "util/log/channel/channel.h"

namespace meta {
namespace channel {

DeviceChannel GetChannel(const string& str) {
  // Default case.
  if (str.empty()) return kChannelDesktopWeb;

  static const unordered_map<string, DeviceChannel> mapping = {
      {"web", kChannelDesktopWeb},
      // Tablet Options.
      {"tablet-web", kChannelTabletWeb},
      {"tablet-app-ios", kChannelTabletAppIOS},
      {"tablet-app-android", kChannelTabletAppAndroid},
      {"tablet-app-windows", kChannelTabletAppWindows},
      {"tablet-app-other", kChannelTabletAppOther},

      // Mobile Options.
      {"mobile-web", kChannelMobileWeb},
      {"mobile-app-ios", kChannelMobileAppIOS},
      {"mobile-app-android", kChannelMobileAppAndroid},
      {"mobile-app-windows", kChannelMobileAppWindows},
      {"mobile-app-other", kChannelMobileAppOther},

      // (TODO (pramodg, edelman, kaushal, holman): These are deprecated.
      // Should never be removed though. they are need for the cms
      // to analyze old bookings
      {"r77mobileweb", kChannelMobileWeb},
      {"r77mobile", kChannelMobileAppIOS},
      {"r77android", kChannelMobileAppAndroid},
  };
  return ::util::tl::FindWithDefault(mapping, str, kChannelDesktopWeb);
}

}  // namespace channel
}  // namespace meta
