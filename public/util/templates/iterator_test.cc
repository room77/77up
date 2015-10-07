// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)

#include "util/templates/iterator.h"

#include <vector>

#include "test/cc/test_main.h"

namespace test {

using ::util::tl::circular_iterator;
using ::util::tl::circular_range;

TEST(Iterator, Sanity) {
 int count;
 vector<int> a;
 int size = 10;
 a.reserve(size);
 for (int i = 0; i < size; ++i)
   a.push_back(i);

 circular_iterator<vector<int>> iter_begin(a);
 circular_iterator<vector<int>> iter_end(a, a.end());

 circular_iterator<vector<int>> iter_1(a, a.begin() + 1);
 circular_iterator<vector<int>> iter_2(a, a.begin() + 2);

 VLOG(3) << "Testing equal iterator.";
 count = 0;
 for(circular_iterator<vector<int>> iter = iter_1; iter != iter_1; ++iter) {
   VLOG(3) << *iter;
   ++count;
 }
 EXPECT_EQ(0, count);

 VLOG(3) << "Testing begin to end. (end is promoted to begin).";
 count = 0;
 for(circular_iterator<vector<int>> iter = iter_begin; iter != iter_end; ++iter){
   VLOG(3) << *iter;
   ++count;
 }
 EXPECT_EQ(0, count);

 VLOG(3) << "Testing intermediate vals.";
 count = 0;
 for(circular_iterator<vector<int>> iter = iter_1; iter != iter_2; ++iter){
   VLOG(3) << *iter;
   ++count;
 }
 EXPECT_EQ(1, count);

 VLOG(3) << "Testing almost complete circular loop. (This is the max we can "
     "hope to get of the iterator.)";
 count = 0;
 for(circular_iterator<vector<int>> iter = iter_2; iter != iter_1; ++iter){
   VLOG(3) << *iter;
   ++count;
 }
 EXPECT_EQ(size - 1, count);

 VLOG(3) << "Testing case when end is accessed.";
 count = 0;
 for(circular_iterator<vector<int>> iter = iter_end; iter != iter_1; ++iter){
   VLOG(3) << *iter;
   ++count;
 }
 EXPECT_EQ(1, count);
}


TEST(Range, Sanity) {
 int count;
 vector<int> a;
 int size = 10;
 a.reserve(size);
 for (int i = 0; i < size; ++i)
   a.push_back(i);


 VLOG(3) << "Testing equal iterator.";
 count = 0;
 for(circular_range<vector<int>> range(a, a.begin() + 1, a.begin() + 1);
     range != range.end(); ++range) {
   VLOG(3) << *range;
   ++count;
 }
 EXPECT_EQ(10, count);

 VLOG(3) << "Testing begin to end. (end is promoted to begin).";
 count = 0;
 for(circular_range<vector<int>> range(a, a.begin(), a.end());
     range != range.range_end(); ++range) {
   VLOG(3) << *range;
   ++count;
 }
 EXPECT_EQ(10, count);

 circular_iterator<vector<int>> iter_1(a, a.begin() + 1);
 circular_iterator<vector<int>> iter_2(a, a.begin() + 2);

 VLOG(3) << "Testing intermediate vals.";
 count = 0;
 for(circular_range<vector<int>> range(iter_1, iter_2);
     range != range.end(); ++range) {
   VLOG(3) << *range;
   ++count;
 }
 EXPECT_EQ(1, count);

 VLOG(3) << "Testing almost complete circular loop.";
 count = 0;
 for(circular_range<vector<int>> range(iter_2, iter_1);
     range != range.range_end(); ++range) {
   VLOG(3) << *range;
   ++count;
 }
 EXPECT_EQ(9, count);

 VLOG(3) << "Testing complete circular loop.";
 count = 0;
 for(circular_range<vector<int>> range(iter_1, iter_1);
     range != range.range_end(); ++range) {
   VLOG(3) << *range;
   ++count;
 }
 EXPECT_EQ(10, count);
}

}  // namespace test
