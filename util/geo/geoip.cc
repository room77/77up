// Copyright 2012 Room77, Inc.
// Author: B. Uygar Oztekin

#include "base/common.h"
#include "util/init/init.h"
#include "geoip.h"

FLAG_string(geoip_database_file, "/home/share/data/geoip_static/GeoIP.dat",
    "Geo IP database typically copied from /usr/share/GeoIP/GeoIP.dat in "
    "debian systems.");

INIT_ADD("geoip", [](){ geoip::GeoIp(); });
