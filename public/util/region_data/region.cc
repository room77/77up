// Copyright 2012 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/region_data/region.h"

FLAG_int(region_lat_grid_size, 5000000,
         "Latitude grid size for region.");
FLAG_int(region_long_grid_size, 6500000,
         "Longitude grid size for region.");
FLAG_int(region_latlong_max_res, 15,
         "Maximum results returned for lat long search.");
FLAG_int(region_min_prefix_word_length, 4,
         "Minimum length a word must have for it to be prefix indexed.");
FLAG_int(region_name_index_max_items_per_key, 15,
         "The maximum number of items stored per key during indexing by name.");
FLAG_int(region_latlong_index_max_items_per_key, -1,
         "The maximum number of items stored per key during indexing by lat long.");
FLAG_int(region_prefix_index_max_items_per_key, 15,
         "The maximum number of items stored per key during indexing by prefix.");
