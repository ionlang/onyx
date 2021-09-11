#### ilc

ilc (Ionlang command-line utility) is a CLI tool for compiling Ion
code.

#### Requirements

* [ionir](https://github.com/ionlang/ionir)
* [ionlang](https://github.com/ionlang/ionlang)
* [CMake](https://cmake.org/download/)
* GCC `>=v10`
* [LLVM](https://releases.llvm.org/download.html)¹ `=v12.0.1`

---
1. _LLVM must be built from source on Windows. A different, close version of LLVM
might work, but you will need to modify `CMakeLists.txt`, specifically where
`find_package(LLVM X.0.0 REQUIRED CONFIG)` occurs._

#### Building

```shell
# Clone the repository.¹
$ git clone https://github.com/ionlang
$ cd ionlang
$ git submodule update --init --recursive

# Generate Makefiles and build.
$ cmake -S ./ -B build/
$ cmake --build build/
```

---
1. _Make sure you've selected the correct branch you're intending to
build (`dev` for latest changes, `master` for stable).

#### Common problems

* *Imported target "x::x" includes non-existent path "/x"*

    Windows:

    1. Delete all installation directories (from both `Program Files` and `Program Files (x86)` of `ionir`).
    2. Re-install the `ionir` project.
    3. Reload the project.
