// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/log/channel/channel.h"
#include "test/cc/test_main.h"

namespace meta {
namespace channel {
namespace test {

TEST(GetChannel, Sanity) {
  EXPECT_EQ(kChannelDesktopWeb, GetChannel(""));
  EXPECT_EQ(kChannelDesktopWeb, GetChannel("web"));
  EXPECT_EQ(kChannelDesktopWeb, GetChannel("asdsad"));
  EXPECT_EQ(kChannelDesktopWeb, GetChannel("invalid"));
  EXPECT_EQ(kChannelTabletWeb, GetChannel("tablet-web"));
  EXPECT_EQ(kChannelMobileWeb, GetChannel("mobile-web"));
  EXPECT_EQ(kChannelMobileWeb, GetChannel("r77mobileweb"));

  EXPECT_EQ(kChannelDesktopWeb, GetChannel({{"channel", ""}}));
  EXPECT_EQ(kChannelDesktopWeb, GetChannel({{"channel", "web"}}));
  EXPECT_EQ(kChannelDesktopWeb, GetChannel({{"channel", "asdsad"}}));
  EXPECT_EQ(kChannelDesktopWeb, GetChannel({{"channel", "invalid"}}));
  EXPECT_EQ(kChannelTabletWeb, GetChannel({{"channel", "tablet-web"}}));
  EXPECT_EQ(kChannelMobileWeb, GetChannel({{"channel", "mobile-web"}}));
  EXPECT_EQ(kChannelMobileWeb, GetChannel({{"channel", "r77mobileweb"}}));
}

TEST(IsWeb, Sanity) {
  EXPECT_TRUE(IsWeb(kChannelDesktopWeb));
  EXPECT_TRUE(IsWeb(kChannelTabletWeb));
  EXPECT_TRUE(IsWeb(kChannelTabletAppIOS));
  EXPECT_TRUE(IsWeb(kChannelTabletAppAndroid));
  EXPECT_TRUE(IsWeb(kChannelTabletAppWindows));
  EXPECT_TRUE(IsWeb(kChannelTabletAppOther));

  EXPECT_FALSE(IsWeb(kChannelMobileWeb));
  EXPECT_FALSE(IsWeb(kChannelMobileAppIOS));
  EXPECT_FALSE(IsWeb(kChannelMobileAppAndroid));
  EXPECT_FALSE(IsWeb(kChannelMobileAppWindows));
  EXPECT_FALSE(IsWeb(kChannelMobileAppOther));
}

TEST(IsDesktop, Sanity) {
  EXPECT_TRUE(IsDesktop(kChannelDesktopWeb));
  EXPECT_FALSE(IsDesktop(kChannelTabletWeb));
  EXPECT_FALSE(IsDesktop(kChannelMobileWeb));
}

TEST(IsTablet, Sanity) {
  EXPECT_TRUE(IsTablet(kChannelTabletWeb));
  EXPECT_TRUE(IsTablet(kChannelTabletAppIOS));
  EXPECT_TRUE(IsTablet(kChannelTabletAppAndroid));
  EXPECT_TRUE(IsTablet(kChannelTabletAppWindows));
  EXPECT_TRUE(IsTablet(kChannelTabletAppOther));

  EXPECT_FALSE(IsTablet(kChannelDesktopWeb));
  EXPECT_FALSE(IsTablet(kChannelMobileWeb));
  EXPECT_FALSE(IsTablet(kChannelMobileAppIOS));
  EXPECT_FALSE(IsTablet(kChannelMobileAppAndroid));
  EXPECT_FALSE(IsTablet(kChannelMobileAppWindows));
  EXPECT_FALSE(IsTablet(kChannelMobileAppOther));
}

TEST(IsTabletApp, Sanity) {
  EXPECT_TRUE(IsTabletApp(kChannelTabletAppIOS));
  EXPECT_TRUE(IsTabletApp(kChannelTabletAppAndroid));
  EXPECT_TRUE(IsTabletApp(kChannelTabletAppWindows));
  EXPECT_TRUE(IsTabletApp(kChannelTabletAppOther));

  EXPECT_FALSE(IsTabletApp(kChannelTabletWeb));
  EXPECT_FALSE(IsTabletApp(kChannelDesktopWeb));
  EXPECT_FALSE(IsTabletApp(kChannelMobileWeb));
  EXPECT_FALSE(IsTabletApp(kChannelMobileAppIOS));
  EXPECT_FALSE(IsTabletApp(kChannelMobileAppAndroid));
  EXPECT_FALSE(IsTabletApp(kChannelMobileAppWindows));
  EXPECT_FALSE(IsTabletApp(kChannelMobileAppOther));
}

TEST(IsMobile, Sanity) {
  EXPECT_TRUE(IsMobile(kChannelMobileWeb));
  EXPECT_TRUE(IsMobile(kChannelMobileAppIOS));
  EXPECT_TRUE(IsMobile(kChannelMobileAppAndroid));
  EXPECT_TRUE(IsMobile(kChannelMobileAppWindows));
  EXPECT_TRUE(IsMobile(kChannelMobileAppOther));

  EXPECT_FALSE(IsMobile(kChannelDesktopWeb));
  EXPECT_FALSE(IsMobile(kChannelTabletWeb));
  EXPECT_FALSE(IsMobile(kChannelTabletAppIOS));
  EXPECT_FALSE(IsMobile(kChannelTabletAppAndroid));
  EXPECT_FALSE(IsMobile(kChannelTabletAppWindows));
  EXPECT_FALSE(IsMobile(kChannelTabletAppOther));
}

TEST(IsMobileApp, Sanity) {
  EXPECT_TRUE(IsMobileApp(kChannelMobileAppIOS));
  EXPECT_TRUE(IsMobileApp(kChannelMobileAppAndroid));
  EXPECT_TRUE(IsMobileApp(kChannelMobileAppWindows));
  EXPECT_TRUE(IsMobileApp(kChannelMobileAppOther));

  EXPECT_FALSE(IsMobileApp(kChannelMobileWeb));
  EXPECT_FALSE(IsMobileApp(kChannelDesktopWeb));
  EXPECT_FALSE(IsMobileApp(kChannelTabletWeb));
  EXPECT_FALSE(IsMobileApp(kChannelTabletAppIOS));
  EXPECT_FALSE(IsMobileApp(kChannelTabletAppAndroid));
  EXPECT_FALSE(IsMobileApp(kChannelTabletAppWindows));
  EXPECT_FALSE(IsMobileApp(kChannelTabletAppOther));
}

}  // namespace test
}  // namespace channel
}  // namespace meta
