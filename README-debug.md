# Velox Debug Build + VSCode/GDB Setup (RHEL 9)

This README documents a working **Debug** build of **Velox** (with `env -i`), how to build and run `velox_in_10_min_demo`, and how to set up **VSCode + gdb** with a convenient `vvec` command for printing Velox vectors.

> Paths below assume your repo is at: `/data/lhvu/projects/velox`  
> Adjust paths if your checkout is elsewhere.

---

## 1) Repo layout (expected)

From the repo root:

```bash
cd /data/lhvu/projects/velox
ls
```

You should see (among other files):

- `CMakeLists.txt`
- `velox/` (source)
- `build-main-debug/` (your Debug build dir)
- `compile_commands.json` (symlink or file; recommended)

---

## 2) Configure a Debug build with `env -i`

Create a clean build directory:

```bash
cd /data/lhvu/projects/velox
mkdir -p build-main-debug
cd build-main-debug
```

Configure with a minimal environment and Debug flags:

```bash
env -i   PATH=/usr/local/cmake/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin   HOME=$HOME   CC=/usr/bin/gcc   CXX=/usr/bin/g++   cmake .. -GNinja     -DCMAKE_BUILD_TYPE=Debug     -DCMAKE_EXPORT_COMPILE_COMMANDS=ON     -DVELOX_BUILD_TESTING=ON     -DBoost_NO_SYSTEM_PATHS=ON     -DBOOST_ROOT=/usr     -DBOOST_INCLUDEDIR=/usr/include     -DBOOST_LIBRARYDIR=/usr/lib64     -DCMAKE_EXE_LINKER_FLAGS="-latomic"     -DCMAKE_SHARED_LINKER_FLAGS="-latomic"     -DCMAKE_MODULE_LINKER_FLAGS="-latomic"
```

### Optional: keep a top-level `compile_commands.json` (recommended for VSCode)
From the repo root:

```bash
cd /data/lhvu/projects/velox
ln -sf build-main-debug/compile_commands.json ./compile_commands.json
```

---

## 3) Build the demo target

From the build directory:

```bash
cd /data/lhvu/projects/velox/build-main-debug
ninja velox_in_10_min_demo
```

---

## 4) Run `velox_in_10_min_demo`

From the build directory:

```bash
cd /data/lhvu/projects/velox/build-main-debug
./velox/exec/tests/velox_in_10_min_demo
```

---

## 5) Verify debug symbols are present

### Quick check
```bash
file /data/lhvu/projects/velox/build-main-debug/velox/exec/tests/velox_in_10_min_demo
```

You want to see `with debug_info` and `not stripped`.

### DWARF section check
```bash
readelf -S /data/lhvu/projects/velox/build-main-debug/velox/exec/tests/velox_in_10_min_demo | grep -E '\.debug|\.zdebug'
```

---

## 6) GDB helper: `vvec` to print vector slices

When debugging Velox vectors in gdb/VSCode, printing `std::string` return values can be noisy.
This helper uses `toString(start,count).c_str()` and prints it immediately.

### 6.1 Create the gdb commands file

Create:

```bash
mkdir -p /data/lhvu/projects/velox/.gdb
```

Save this as:

**`/data/lhvu/projects/velox/.gdb/velox_cmds.gdb`**

```gdb
define vvec
  # usage: vvec <expr> <start> <count>
  # Example: vvec b 1 5
  # Prints the slice using BaseVector::toString(start,count).c_str()
  x/s ((facebook::velox::BaseVector*)($arg0).get())->toString((facebook::velox::vector_size_t)$arg1, (facebook::velox::vector_size_t)$arg2).c_str()
end

document vvec
Print a Velox VectorPtr slice.
Usage: vvec <expr> <start> <count>
Example: vvec b 1 5
end
```

### 6.2 Use in gdb (CLI)

```bash
gdb -q /data/lhvu/projects/velox/build-main-debug/velox/exec/tests/velox_in_10_min_demo
```

Inside gdb:

```gdb
source /data/lhvu/projects/velox/.gdb/velox_cmds.gdb
break VeloxIn10MinDemo::run
run
vvec b 1 5
```

---

## 7) VSCode setup (launch.json + settings.json)

### 7.1 Create `.vscode/` at repo root

```bash
cd /data/lhvu/projects/velox
mkdir -p .vscode
```

### 7.2 `.vscode/settings.json`

Create **`/data/lhvu/projects/velox/.vscode/settings.json`**:

```json
{
  "C_Cpp.default.compileCommands": "${workspaceFolder}/compile_commands.json",
  "C_Cpp.default.cppStandard": "c++20",
  "C_Cpp.default.intelliSenseMode": "linux-gcc-x64"
}
```

### 7.3 `.vscode/launch.json`

Create **`/data/lhvu/projects/velox/.vscode/launch.json`**:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Velox: velox_in_10_min_demo (gdb)",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build-main-debug/velox/exec/tests/velox_in_10_min_demo",
      "args": [],
      "cwd": "${workspaceFolder}/build-main-debug",
      "MIMode": "gdb",
      "miDebuggerPath": "/usr/bin/gdb",
      "stopAtEntry": false,
      "externalConsole": false,
      "environment": [
        { "name": "GLOG_logtostderr", "value": "1" }
      ],
      "setupCommands": [
        { "text": "set debuginfod enabled off" },
        { "text": "set print pretty off" },
        { "text": "set print elements 0" },
        { "text": "set auto-load safe-path /" },
        { "text": "source /data/lhvu/projects/velox/.gdb/velox_cmds.gdb" }
      ]
    }
  ]
}
```

> Notes:
> - `engineLogging` is intentionally **not** enabled to avoid Debug Console noise.
> - In VSCode **Debug Console**, run gdb commands with `-exec ...`.  
>   Example:
>   ```text
>   -exec vvec b 1 5
>   -exec vvec a 0 7
>   ```

---

## 8) Common usage examples

### Print slices
In VSCode Debug Console (must include `-exec`):

```text
-exec vvec a 1 2
-exec vvec b 1 5
```

### Check sizes
```text
-exec p a->size()
-exec p b->size()
```

---

## 9) Optional: suppress “Missing rpms …” hints in gdb

If you see a message like `Missing rpms, try: dnf ...`, you can ignore it (Velox debugging still works).
To suppress it:

```gdb
set debuginfod enabled off
```

This is already included in the VSCode `setupCommands`.

---

## 10) Build + run summary (copy/paste)

```bash
cd /data/lhvu/projects/velox
mkdir -p build-main-debug
cd build-main-debug

env -i   PATH=/usr/local/cmake/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin   HOME=$HOME   CC=/usr/bin/gcc   CXX=/usr/bin/g++   cmake .. -GNinja     -DCMAKE_BUILD_TYPE=Debug     -DCMAKE_EXPORT_COMPILE_COMMANDS=ON     -DVELOX_BUILD_TESTING=ON     -DBoost_NO_SYSTEM_PATHS=ON     -DBOOST_ROOT=/usr     -DBOOST_INCLUDEDIR=/usr/include     -DBOOST_LIBRARYDIR=/usr/lib64     -DCMAKE_EXE_LINKER_FLAGS="-latomic"     -DCMAKE_SHARED_LINKER_FLAGS="-latomic"     -DCMAKE_MODULE_LINKER_FLAGS="-latomic"

ninja velox_in_10_min_demo
./velox/exec/tests/velox_in_10_min_demo
```
