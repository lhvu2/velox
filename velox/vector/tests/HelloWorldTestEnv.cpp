#include <gtest/gtest.h>
#include <mutex>
#include "velox/common/memory/Memory.h"

using namespace facebook::velox;

namespace {
std::once_flag kInitVeloxMemoryOnce;

class VeloxMemoryManagerEnv : public ::testing::Environment {
 public:
  void SetUp() override {
    std::call_once(kInitVeloxMemoryOnce, []() {
      memory::MemoryManager::initialize({});
    });
  }
};
} // namespace

static ::testing::Environment* const kVeloxMemEnv =
    ::testing::AddGlobalTestEnvironment(new VeloxMemoryManagerEnv());
