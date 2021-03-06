// Copyright 2015, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold (buchhold@informatik.uni-freiburg.de)

#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include "../src/engine/Engine.h"


TEST(EngineTest, joinTest) {
  Engine e;
  vector<array<Id, 2>> a;
  a.push_back(array<Id, 2>{{1, 1}});
  a.push_back(array<Id, 2>{{1, 3}});
  a.push_back(array<Id, 2>{{2, 1}});
  a.push_back(array<Id, 2>{{2, 2}});
  a.push_back(array<Id, 2>{{4, 1}});
  vector<array<Id, 2>> b;
  b.push_back(array<Id, 2>{{1, 3}});
  b.push_back(array<Id, 2>{{1, 8}});
  b.push_back(array<Id, 2>{{3, 1}});
  b.push_back(array<Id, 2>{{4, 2}});
  vector<array<Id, 3>> res;
  e.join(a, 0, b, 0, &res);


  ASSERT_EQ(1u, res[0][0]);
  ASSERT_EQ(1u, res[0][1]);
  ASSERT_EQ(3u, res[0][2]);

  ASSERT_EQ(1u, res[1][0]);
  ASSERT_EQ(1u, res[1][1]);
  ASSERT_EQ(8u, res[1][2]);

  ASSERT_EQ(1u, res[2][0]);
  ASSERT_EQ(3u, res[2][1]);
  ASSERT_EQ(3u, res[2][2]);

  ASSERT_EQ(1u, res[3][0]);
  ASSERT_EQ(3u, res[3][1]);
  ASSERT_EQ(8u, res[3][2]);

  ASSERT_EQ(5u, res.size());

  ASSERT_EQ(4u, res[4][0]);
  ASSERT_EQ(1u, res[4][1]);
  ASSERT_EQ(2u, res[4][2]);

  res.clear();
  for (size_t i = 1; i <= 10000; ++i) {
    b.push_back(array<Id, 2>{{4 + i, 2 + i}});
  }
  a.push_back(array<Id, 2>{{400000, 200000}});
  b.push_back(array<Id, 2>{{400000, 200000}});
  e.join(a, 0, b, 0, &res);
  ASSERT_EQ(6u, res.size());

  a.clear();
  b.clear();
  res.clear();

  for (size_t i = 1; i <= 10000; ++i) {
    a.push_back(array<Id, 2>{{4 + i, 2 + i}});
  }
  a.push_back(array<Id, 2>{{40000, 200000}});
  b.push_back(array<Id, 2>{{40000, 200000}});

  for (size_t i = 1; i <= 10000; ++i) {
    a.push_back(array<Id, 2>{{40000 + i, 2 + i}});
  }
  a.push_back(array<Id, 2>{{4000001, 200000}});
  b.push_back(array<Id, 2>{{4000001, 200000}});
  e.join(a, 0, b, 0, &res);
  ASSERT_EQ(2u, res.size());
};

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

