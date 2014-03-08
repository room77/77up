#include "base/common.h"
#include "util/memory/stringstorage.h"
#include "util/memory/stringstorage_unlock.h"
#include "test/cc/test_main.h"
#include "util/index/index.h"

#include "collection.h"

typedef struct {
  int x, y;
  string z;
  SIGNATURE(x*1 / y*2 / z*3);
} SimpleTuple;

TEST(Collection, Sanity) {
  //
  // Test UniqueCollection class
  //

  UniqueCollection<SimpleTuple> coll;

  SimpleTuple a;

  a.x = 1;
  a.y = 2;
  a.z = "hello";

  SimpleTuple b = a;

  // tuples "a" and "b" are now identical

  const SimpleTuple *item1 = coll.Store(a);
  const SimpleTuple *item2 = coll.Store(b);
  const SimpleTuple *item3 = coll.Store(a);

  // even though "a" and "b" are stored repeatedly, there should only be
  // one copy inside the collection.

  ASSERT(item1 == item2);
  ASSERT(item1 == item3);
  ASSERT(coll.size() == 1);

  // now let's change "b"
  b.x = 10;
  const SimpleTuple *item4 = coll.Store(b);  // a new item is stored
  ASSERT(item1 != item4);

  // change "b" again
  b.x = 100;
  b.z = "world";
  const SimpleTuple *item5 = coll.Store(b);  // a new item is stored
  ASSERT(item1 != item5);

  // let's store "a" again
  const SimpleTuple *item6 = coll.Store(a);
  ASSERT(item1 == item6);

  // there should be 3 items in the collection
  ASSERT(coll.size() == 3);

  // test const_iterator
  int count = 0;
  for (UniqueCollection<SimpleTuple>::const_iterator i = coll.begin();
       i != coll.end();
       ++i) {
    const SimpleTuple *t = *i;
    LOG(INFO) << "*** " << t->x << " " << t->y << " " << t->z;
    ASSERT(t->y == 2);
    count++;
  }
  ASSERT(count == coll.size());

  // now test Index on UniqueCollection

  // an index on field "x" (int)
  Index<int, const SimpleTuple *> index1a;
  BUILD_INDEX_ON_FIELD(coll, x, index1a);
  const SimpleTuple *search_result1 = index1a.RetrieveUnique(10);
  LOG(INFO) << "(test 1) retrieved: "
         << search_result1->x << " "
         << search_result1->y << " "
         << search_result1->z;
  ASSERT(search_result1->x == 10 && search_result1->y == 2
         && search_result1->z == "hello");

  ASSERT(index1a.RetrieveUnique(2) == NULL);

  // an index on two fields "y" and "z" (int and string)
  Index<pair<int, const char *>, const SimpleTuple *> index1b;
  BUILD_INDEX_ON_TWO_FIELDS(coll, y, z, index1b);
  const SimpleTuple *search_result1b =
    index1b.RetrieveUnique(pair<int, string>(2, "world"));
  ASSERT(search_result1b != NULL);
  LOG(INFO) << "(test 1b) retrieved: "
         << search_result1b->x << " "
         << search_result1b->y << " "
         << search_result1b->z;
  ASSERT(search_result1b->x == 100 && search_result1b->y == 2
         && search_result1b->z == "world");

  // another index on field "z" (string)
  Index<const char *, const SimpleTuple *> index1;
  BUILD_INDEX_ON_FIELD(coll, z, index1);

  // retrieve from index using Retrieve
  vector<const SimpleTuple *> search_result2;
  index1.Retrieve("hello", &search_result2);
  for (int i = 0; i < search_result2.size(); i++)
    LOG(INFO) << "(test 2) retrieved: "
           << search_result2[i]->x << " "
           << search_result2[i]->y << " "
           << search_result2[i]->z;
  ASSERT(search_result2.size() == 2);

  //
  // Test Collection class
  //

  Collection<SimpleTuple> coll2;

  const SimpleTuple *item7 = coll2.Store(a);
  const SimpleTuple *item8 = coll2.Store(b);
  const SimpleTuple *item9 = coll2.Store(b);
  ASSERT(item7 != item8);
  ASSERT(item8 != item9);
  ASSERT(coll2.size() == 3);

  // test HeavyIndex on Collection
  typedef HeavyIndex<const char *, const SimpleTuple *> tIndexType2;
  tIndexType2 index2;

  BUILD_INDEX_ON_FIELD(coll2, z, index2);

  // retrieve from index using RetrieveSet
  const tIndexType2::tValueSet *search_result3 = index2.RetrieveSet("world");
  for (tIndexType2::tValueSet::const_iterator i = search_result3->begin();
       i != search_result3->end();
       ++i)
    LOG(INFO) << "(test 3) retrieved: "
           << (*i)->x << " " << (*i)->y << " " << (*i)->z;
  ASSERT(search_result3->size() == 2);

  // retrieve from index using Retrieve
  vector<const SimpleTuple *> search_result4;
  index2.Retrieve("hello", &search_result4);
  for (int i = 0; i < search_result4.size(); i++)
    LOG(INFO) << "(test 4) retrieved: "
           << search_result4[i]->x << " "
           << search_result4[i]->y << " "
           << search_result4[i]->z;
  ASSERT(search_result4.size() == 1);

  // test RetrieveUnique
  const SimpleTuple *result2 = index1.RetrieveUnique("world");
  ASSERT(result2 != NULL && result2->x == 100 && result2->z == "world");
  ASSERT(index2.RetrieveUnique("hello123") == NULL);

  LOG(INFO) << "PASS";
}
