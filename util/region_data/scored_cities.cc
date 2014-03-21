// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


#include "util/init/init.h"

extern bool gFlag_region_cities_use_scored_cities;

// Init before cities.
INIT_ADD("scored_cities", -10, []{ gFlag_region_cities_use_scored_cities = true; });
