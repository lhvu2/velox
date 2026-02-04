# Velox build-dev (Debug / RelWithDebInfo) – Minimal, Incremental Workflow

This README documents a **clean, minimal build-dev workflow** for Velox using:
- `env -i` to avoid Conda leakage
- incremental `ninja` builds
- running **vector tests** and **Presto map_filter tests**
- locating where `map_filter` is implemented

This version **removes unnecessary steps** and assumptions you correctly pointed out.

---

## 1. Preconditions (one-time)

- Velox repo already cloned
- Dependencies already installed on your system - lhvu(e.g. via `scripts/setup-centos9.sh`)
- You are **not adding new .cpp files** (only running existing tests)

No assumptions about Conda, INSTALL_PREFIX, or DEPENDENCY_DIR are required here.

---

## 2. Create build-dev (one time)

From the Velox repo root:

```bash
cd /data/lhvu/projects/velox

mkdir -p build-dev
cd build-dev
```

---

## 3. Configure with env -i (clean environment)

Use **env -i** to prevent Conda or shell state from leaking into the build.

```bash
env -i \
  PATH=/usr/local/cmake/bin:/usr/local/bin:/usr/bin:/bin \
  HOME=$HOME \
  CC=/usr/bin/gcc \
  CXX=/usr/bin/g++ \
  cmake .. \
    -GNinja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DVELOX_BUILD_TESTING=ON
```

Notes:
- `RelWithDebInfo` is strongly preferred over `Debug`
- This step is done **once**, unless CMake files change

---

## 4. Incremental builds with ninja

After the initial configure, **never rerun cmake** unless you modify CMakeLists.txt.

### Build vector tests only

```bash
ninja -j2 velox_vector_test
```

### Run vector tests

```bash
./velox/vector/tests/velox_vector_test
```

List tests:
```bash
./velox/vector/tests/velox_vector_test --gtest_list_tests
```

Run a subset:
```bash
./velox/vector/tests/velox_vector_test --gtest_filter='DecodedVectorTest.*'
```

---

## 5. Build and run Presto map_filter tests

### Build Presto SQL function tests

```bash
ninja -j2 velox_functions_test
```

### Run only map_filter tests

```bash
./velox/functions/prestosql/tests/velox_functions_test \
  --gtest_filter='*map_filter*:*MapFilter*'
```

This does **not** rebuild all of Velox — only the affected test target.

---

## 6. Where is map_filter implemented?

### Install ripgrep (once)

```bash
sudo dnf install -y ripgrep
```

### Find all map_filter references

```bash
rg -n map_filter velox/functions/prestosql
```

Key files:

- **Implementation**
  ```
  velox/functions/prestosql/FilterFunctions.cpp
  ```

- **Registration**
  ```
  velox/functions/prestosql/registration/MapFunctionsRegistration.cpp
  ```

- **Tests**
  ```
  velox/functions/prestosql/tests/MapFilterTest.cpp
  ```

There is **no MapFilter.cpp** by design — map-related functions are grouped in shared files.

---

## 7. Incremental workflow summary

| Action | Command |
|------|--------|
Initial configure | `env -i cmake ..`
Rebuild vector tests | `ninja velox_vector_test`
Run vector tests | `./velox/vector/tests/velox_vector_test`
Rebuild Presto functions | `ninja velox_functions_test`
Run map_filter tests | `./velox/functions/prestosql/tests/velox_functions_test --gtest_filter='*MapFilter*'`
Find implementation | `rg map_filter velox/functions/prestosql`

---

## 8. Performance & stability tips

- Avoid `make debug` (it builds *everything*)
- Prefer `ninja -j1` or `-j2` on memory-constrained machines
- Stick to **targeted ninja builds**, not `ninja all`

---

## 9. When do you need to rerun cmake?

You **do NOT** need to rerun cmake when:
- Editing existing `.cpp` files
- Adding new test *cases* inside an existing test file

You **DO** need to rerun cmake only if:
- You add a **new .cpp file**
- You modify any `CMakeLists.txt`

---

## TL;DR (fast path)

```bash
# once
mkdir build-dev && cd build-dev
env -i PATH=/usr/local/cmake/bin:/usr/bin:/bin HOME=$HOME cmake .. -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo

# daily
ninja -j2 velox_functions_test
./velox/functions/prestosql/tests/velox_functions_test --gtest_filter='*MapFilter*'
```

This is the **correct minimal workflow** for developing and testing `map_filter`.
