#include <gtest/gtest.h>

#include "velox/common/memory/Memory.h"
#include "velox/vector/FlatVector.h"
#include "velox/vector/tests/utils/VectorMaker.h"

using namespace facebook::velox;

TEST(HelloWorldTest, basicVector) {
  // MemoryManager is initialized by HelloWorldTestEnv.cpp

  auto pool = memory::memoryManager()->addLeafPool();
  test::VectorMaker maker(pool.get());

  auto vector = maker.flatVector<int32_t>({1, 2, 3});

  ASSERT_EQ(vector->size(), 3);

  auto flat = vector->as<FlatVector<int32_t>>();
  ASSERT_NE(flat, nullptr);

  EXPECT_EQ(flat->valueAt(0), 1);
  EXPECT_EQ(flat->valueAt(1), 2);
  EXPECT_EQ(flat->valueAt(2), 3);
}
