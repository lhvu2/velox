# Debug Build + GDB (TUI) Quickstart

This README is a **copy/paste** guide for:
- Building with a clean environment (`env -i`) and **debug symbols**
- Building **two Ninja targets**
- Running a **single test** from the command line (GoogleTest)
- Starting **GDB** for a specific test binary (with **TUI** on by default)
- Practical GDB commands: breakpoints, run, continue, next/step, frames, backtrace, etc.

> Notes
> - Commands assume you are using **CMake + Ninja**.
> - Examples use Velox-style test binaries, but the workflow is identical for any C/C++ project that uses GoogleTest.

---

## 0) Prereqs

- CMake
- Ninja
- GCC/G++ (or Clang/Clang++)
- GDB installed (Homebrew on macOS or distro packages on Linux)

Verify:
```bash
cmake --version
ninja --version
gdb --version
```

---

## 1) Configure a debug build with `env -i`

Create a clean build directory:
```bash
mkdir -p build-debug
cd build-debug
```

Run CMake with a clean environment (`env -i`). This helps avoid hidden env vars causing link/runtime issues.

### Example configuration (GCC, RelWithDebInfo)
RelWithDebInfo = optimized enough, **but keeps symbols**.
```bash
env -i \
  PATH=/usr/local/bin:/usr/bin:/bin \
  HOME="$HOME" \
  CC=/usr/bin/gcc \
  CXX=/usr/bin/g++ \
  cmake .. -GNinja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DVELOX_BUILD_TESTING=ON \
    -DCMAKE_EXE_LINKER_FLAGS="-latomic" \
    -DCMAKE_SHARED_LINKER_FLAGS="-latomic" \
    -DCMAKE_MODULE_LINKER_FLAGS="-latomic"
```

### If you want *maximum* debuggability (recommended for stepping)
Use `Debug` and disable optimizations:
```bash
env -i \
  PATH=/usr/local/bin:/usr/bin:/bin \
  HOME="$HOME" \
  CC=/usr/bin/gcc \
  CXX=/usr/bin/g++ \
  cmake .. -GNinja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DVELOX_BUILD_TESTING=ON \
    -DCMAKE_C_FLAGS="-O0 -g" \
    -DCMAKE_CXX_FLAGS="-O0 -g"
```

> Tip: stepping in optimized builds can feel “weird” (lines skipped, vars optimized away). If you’re learning/debugging, prefer **Debug + -O0**.

---

## 2) Build with Ninja (two targets)

From inside `build-debug/`:

### Build target #1 (example: vector tests)
```bash
ninja -v velox_vector_test
```

### Build target #2 (example: exec tests)
```bash
ninja -v velox_exec_test
```

> Replace `velox_vector_test` / `velox_exec_test` with the two targets you actually need.

---

## 3) Run tests from the command line

### 3.1 Find the built binary
Common locations (Velox examples):
- `./velox/vector/tests/velox_vector_test`
- `./velox/exec/tests/velox_exec_test`

List likely outputs:
```bash
find . -maxdepth 4 -type f -name '*test*' | head
```

### 3.2 Run **all** tests in a binary
```bash
./velox/vector/tests/velox_vector_test
```

### 3.3 Run a **single test** (GoogleTest filter)
```bash
./velox/vector/tests/velox_vector_test --gtest_filter=HelloWorldTest.*
```

You can also list tests:
```bash
./velox/vector/tests/velox_vector_test --gtest_list_tests
```

---

## 4) Make GDB always start in TUI mode (`~/.gdbinit`)

Create / edit:
```bash
nano ~/.gdbinit
```

Paste:

```gdb
# --- UI / quality-of-life ---
set pagination off
set confirm off
set listsize 12
set style enabled on

# Always start in TUI mode
tui enable

# Always show source around the current execution line when stopping
define hook-stop
  list .
end
```

Save and exit.

### Useful TUI keys
- `Ctrl + x` then `a` : toggle TUI on/off
- `Ctrl + x` then `2` : split view
- `Ctrl + l` : redraw screen

---

## 5) Start GDB for a specific test target

### 5.1 Launch GDB on the test binary
```bash
gdb ./velox/vector/tests/velox_vector_test
```

(With `~/.gdbinit` above, TUI will turn on automatically.)

### 5.2 Pass test args (recommended)
Inside gdb:
```gdb
set args --gtest_filter=HelloWorldTest.*
run
```

Or in one command from the shell:
```bash
gdb --args ./velox/vector/tests/velox_vector_test --gtest_filter=HelloWorldTest.*
```

---

## 6) Breakpoints + basic GDB workflow

### 6.1 Set breakpoints
Break on a function:
```gdb
break isPrime
```

Break on a specific line:
```gdb
break primesProgram_buggy.c:65
```

List breakpoints:
```gdb
info breakpoints
```

Disable/enable a breakpoint:
```gdb
disable 1
enable 1
```

Delete a breakpoint:
```gdb
delete 1
```

Conditional breakpoint (stop only when condition is true):
```gdb
break getPrimes if n == 20
```

Temporary breakpoint:
```gdb
tbreak main
```

### 6.2 Run / continue execution
Start program:
```gdb
run
```

Run to next breakpoint:
```gdb
continue
# or: c
```

### 6.3 Step control
Step **into** a function call:
```gdb
step
# or: s
```

Step **over** a function call:
```gdb
next
# or: n
```

Finish current function (run until it returns):
```gdb
finish
```

Run until a specific line in the current frame:
```gdb
until 120
# or: advance 120
```

### 6.4 Inspect variables
Print a variable:
```gdb
print x
# or: p x
```

Print in hex:
```gdb
p/x x
```

Show arguments and locals in the current frame:
```gdb
info args
info locals
```

Watchpoint (stop when a variable changes):
```gdb
watch total
```

### 6.5 Stack trace, frames, and moving around
Backtrace:
```gdb
bt
# where is usually the same as bt
where
```

Show current frame:
```gdb
frame
```

Select a different frame:
```gdb
frame 1
frame 2
```

Move up/down the call stack:
```gdb
up
down
```

After selecting a frame, `print` will refer to that frame’s locals/args.

### 6.6 Source listing (cursor vs scrolling)
Show code around current execution point:
```gdb
list .
# (IMPORTANT: this forces listing around the current cursor)
```

Show next listing chunk:
```gdb
list
```

Show previous listing chunk:
```gdb
list -
```

---

## 7) Fast “debug a single test” recipe

From `build-debug/`:

1) Build the binary:
```bash
ninja -v velox_vector_test
```

2) Run just one test normally:
```bash
./velox/vector/tests/velox_vector_test --gtest_filter=HelloWorldTest.*
```

3) Run the same test under gdb:
```bash
gdb --args ./velox/vector/tests/velox_vector_test --gtest_filter=HelloWorldTest.*
```

Inside gdb:
```gdb
break HelloWorldTest_HelloWorld_Test::TestBody
run
bt
next
step
continue
```

> Tip: Depending on how the test is compiled, the exact symbol name for `TestBody` can vary.
> If `break ...TestBody` doesn’t work, set a breakpoint in a function you know is executed, or break by file:line.

---

## 8) Common troubleshooting

### “undefined reference to `sqrt`”
Link the math library:
```bash
gcc -Wall -g primesProgram_buggy.c -lm -o primes
```

### “Cannot enable the TUI when output is not a terminal”
You’re likely running gdb in a non-interactive environment or with redirected output.
Run it in a real terminal, and ensure stdin/stdout are a TTY.

### macOS: gdb permissions / codesign
Homebrew gdb on macOS requires codesigning and “Developer Tools” permission.
See the GDB Darwin permissions guide:
https://sourceware.org/gdb/wiki/PermissionsDarwin

---

## 9) Handy command cheat sheet

Inside GDB:

| Goal | Command |
|---|---|
| Start program | `run` |
| Run to next breakpoint | `continue` / `c` |
| Step into | `step` / `s` |
| Step over | `next` / `n` |
| Run until return | `finish` |
| Break at function | `break foo` |
| Break at file:line | `break file.c:123` |
| List breakpoints | `info breakpoints` |
| Print variable | `print x` |
| Show locals | `info locals` |
| Stack trace | `bt` / `where` |
| Select stack frame | `frame N` / `up` / `down` |
| Show code at cursor | `list .` |
| Quit | `quit` |

---

## 10) Fill in your project-specific bits

Replace these with your actual values:
- Build directory: `build-debug`
- Targets: `velox_vector_test`, `velox_exec_test`
- Test binary paths: `./velox/.../velox_*_test`
- Test filter: `--gtest_filter=...`

That’s all you need for a tight “build → run → debug” loop.
