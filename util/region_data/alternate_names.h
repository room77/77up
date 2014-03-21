// Copyright 2013 Room77, Inc.
// Author: Sungsoon Cho

#ifndef _UTIL_REGION_DATA_ALTERNATE_NAMES_H_
#define _UTIL_REGION_DATA_ALTERNATE_NAMES_H_

#include "base/defs.h"
#include "util/region_data/attractions.h"
#include "util/region_data/cities.h"
#include "util/region_data/region.h"

namespace region_data { namespace alternate_names {

string ExtractNonStopWords(const string& orig);
int ExtractNonStopWords(const string& orig, string *p_ret);

void RewriteAlternateCityNames(tCity *p_city);

string NormalizeName(const string& name);

void WriteNormalizedAndAlternateCityNames(tCity *p_region);

void WriteNormalizedAndAlternateAttractionNames(tAttraction *p_region);

}  // namespace alternate_names
}  // namespace region_data

#endif

