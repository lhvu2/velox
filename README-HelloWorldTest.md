# Velox Hello World Test Setup (DuckDB Disabled)

This document summarizes the **working build configuration, commands, and test setup**
used to build Velox, disable DuckDB, and run a minimal “Hello World” GTest using
Velox vectors.

This setup is validated on **RHEL 9 / Rocky / Alma**-style systems and avoids
common issues with DuckDB, Folly, and `std::atomic`.

---

## Goals

- Build Velox successfully
- Disable DuckDB (not needed for Substrait / semantic operators)
- Add a minimal HelloWorld GTest
- Initialize Velox `MemoryManager` correctly
- Use a clean, reproducible build directory
- Run tests reliably via both GTest and CTest

---

## Repository Layout

```
velox/
├── CMakeLists.txt
├── CMake/
├── velox/
│   └── vector/
│       └── tests/
│           ├── HelloWorldTestEnv.cpp
│           ├── HelloWorldTest.cpp
│           └── CMakeLists.txt
└── build-test/        # out-of-tree build directory
```

---

## 1. Disable DuckDB

This Velox tree **unconditionally builds DuckDB** via FetchContent.
There is no CMake option to turn it off, so it must be disabled manually.

### Edit top-level `CMakeLists.txt`

```bash
vi CMakeLists.txt
```

Comment out the DuckDB dependency line (exact location may vary):

```cmake
262 # velox_resolve_dependency(DUCKDB)
```

---

## 2. HelloWorld Test Environment

Velox requires the `MemoryManager` to be initialized **before any test runs**.
This is done once via a **global GTest environment**.

### `HelloWorldTestEnv.cpp`

```cpp
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
```

---

## 3. HelloWorld Test

### `HelloWorldTest.cpp`

```cpp
#include <gtest/gtest.h>
#include "velox/common/memory/Memory.h"
#include "velox/vector/FlatVector.h"
#include "velox/vector/tests/utils/VectorMaker.h"

using namespace facebook::velox;

TEST(HelloWorldTest, basicVector) {
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
```

---

## 4. Register the Test in CMake

Edit `velox/vector/tests/CMakeLists.txt` and add:

```cmake
HelloWorldTestEnv.cpp
HelloWorldTest.cpp
```

to the `velox_vector_test` target sources.

---

## 5. Create Build Directory

```bash
cd velox
rm -rf build-test
mkdir build-test
cd build-test
```

---

## 6. Configure (with atomic fix)

```bash
env -i \
  PATH=/usr/local/cmake/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin \
  HOME=$HOME \
  CC=/usr/bin/gcc \
  CXX=/usr/bin/g++ \
  cmake .. -GNinja \
    -DVELOX_BUILD_TESTING=ON \
    -DCMAKE_EXE_LINKER_FLAGS="-latomic" \
    -DCMAKE_SHARED_LINKER_FLAGS="-latomic" \
    -DCMAKE_MODULE_LINKER_FLAGS="-latomic"
```

---

## 7. Build

```bash
ninja velox_vector_test
```

---

## 8. Run Tests

```bash
./velox/vector/tests/velox_vector_test --gtest_filter=HelloWorldTest.*
```

or

```bash
ctest -R velox_vector_test --output-on-failure
```

---

## Result

You now have a stable Velox development setup suitable for:
- Substrait plan execution
- Custom semantic operators
- Incremental testing
