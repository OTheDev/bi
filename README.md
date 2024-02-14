# bi
[![](https://github.com/OTheDev/bi/actions/workflows/multiplatform_test.yml/badge.svg)](https://github.com/OTheDev/bi/actions/workflows/multiplatform_test.yml)
[![](https://github.com/OTheDev/bi/actions/workflows/doc.yml/badge.svg)](https://github.com/OTheDev/bi/actions/workflows/doc.yml)
[![](https://github.com/OTheDev/bi/actions/workflows/static_analysis.yml/badge.svg)](https://github.com/OTheDev/bi/actions/workflows/static_analysis.yml)

`bi` is an arbitrary precision integer library for C++.

## Table of Contents

- [Documentation](#documentation)
- [Building from Source](#building-from-source)
- [Finding and Linking Against the Library Using find_package()](#finding-and-linking-against-the-library-using-find_package)
- [Packaging with CPack](#packaging-with-cpack)

## Documentation

For detailed documentation, including this document and an
[API reference](https://othedev.github.io/bi/classbi_1_1bi__t.html), visit
the [`bi` Documentation](https://OTheDev.github.io/bi/).

## Building from Source

### Prerequisites

- **CMake** (version 3.15 or higher): Required for configuring and building the
   project.
- **C++20** compiler.

### Steps to Build

1. Clone the [repository](https://github.com/othedev/bi):

   ```shell
   git clone <repository-url-or-ssh>
   cd bi
   ```

2. Create a build directory:

   ```shell
   mkdir build && cd build
   ```

3. Configure the build:

   For the default configuration:

   ```shell
   cmake ..
   ```

   For custom configurations, refer to the [Options](#options) section below.

   **Note**: The `bi` library targets C++20. Sometimes, the default compiler
   found by CMake might lack support for some C++20 features used in the
   library. CMake allows you to specify the compiler manually. For instance:

   ```shell
   cmake .. -DCMAKE_CXX_COMPILER=clang++
   ```

   More generally:

   ```shell
   cmake .. -DCMAKE_CXX_COMPILER=/path/to/your/compiler
   ```

4. Build the library

   ```shell
   cmake --build .
   ```

5. Run Tests (if applicable):

   If `BUILD_TESTS` is enabled (default is `ON`), run the tests using:

   ```shell
   ctest
   ```

6. Install the Library:

   ```shell
   cmake --install . --prefix /path/to/install
   ```

   Replace `/path/to/install` with the desired installation directory.

### Options

Configure the build by appending these options to the `cmake` command in step 3:

- `BUILD_SHARED_LIBS`: Build shared libraries (`ON`/`OFF`). Default is `OFF`.
- `BUILD_TESTS`: Build the tests (`ON`/`OFF`). Default is `ON`.

For a **release-optimized build**, set `CMAKE_BUILD_TYPE` to `Release`.

For **development**, enabling the export of compile commands is useful for
tools like linters or editors. To do this, set
`CMAKE_EXPORT_COMPILE_COMMANDS` to `ON`. For example:

```shell
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

## Finding and Linking Against the Library Using `find_package()`

To integrate the `bi` library into a CMake project, locate and link against it
using CMake's `find_package()` command.

1. **Install the `bi` Library** or **Use a Packaged Version of the Library**

- **Install the `bi` Library**

   Ensure that `bi` is installed on your system. If you haven't already installed
   it, or if you're unsure how to do so, refer to the
   [Building From Source](#building-from-source) section for steps on
   creating a build directory, configuring, building, and installing the
   library.

- **Use a Packaged Version of the Library**:

   Alternatively, if you have a CPack-generated package of the `bi` library,
   extract it to a preferred location. This option is useful if you
   prefer not to build from source or are distributing the library to
   others.

   **Important Note**:
   If you install the library or extract a packaged version
   of it to a non-standard location (i.e., not in the default
   system paths), note the path (e.g., `/path/to/install`). You
   may need this information for setting the `CMAKE_PREFIX_PATH` in your project,
   enabling CMake to find and link the library.

2. **Configure Your Project to Find the `bi` Library**

   In your project's `CMakeLists.txt` file, use the `find_package()` command to locate
   the `bi` library:

   ```cmake
   find_package(bi REQUIRED)
   ```

3. **Link Against the `bi` Library**

   After finding the `bi` library with `find_package()`, link it to your target:

   ```cmake
   add_executable(sample sample.cpp)
   target_link_libraries(sample PRIVATE bi::bi)
   ```

4. **Setting the `CMAKE_PREFIX_PATH` (if necessary)**

   This step is only necessary if you installed or extracted `bi` to a
   non-standard location, say, `/path/to/install`.

- **Option 1: Using Command Line**

   Set `CMAKE_PREFIX_PATH` to `/path/to/install` when configuring your project.

   For example:

   ```shell
   cmake .. -DCMAKE_PREFIX_PATH=/path/to/install
   ```

- **Option 2: Modifying `CMakeLists.txt`**

   Alternatively, append the path to `CMAKE_PREFIX_PATH` in your project's
   `CMakeLists.txt`:

   ```cmake
   list(APPEND CMAKE_PREFIX_PATH /path/to/install)
   ```

   Add this before the `find_package()` call.

## Packaging with CPack

After building the `bi` library, one can create a distributable package using
CPack, containing the compiled binaries and necessary headers.

### Creating a Package

1. Complete the build process as described in [Building from Source](#building-from-source).
2. In the build directory, run:

   ```
   cpack
   ```

   Refer to the [`cpack` documentation](https://cmake.org/cmake/help/latest/manual/cpack.1.html#manual:cpack(1))
   for additional options and details.

### Using the Packaged `bi` Library in Projects

Users with compatible platforms can integrate the packaged `bi` library into
their projects.

For example, for CMake-based projects:

1. **Extract the Package**: After downloading or receiving the CPack-generated
   package, extract it to a preferred location on your system.
2. **Configure the Project**: In your CMake project, inform CMake where to find
   the `bi` library by setting the `CMAKE_PREFIX_PATH` to the path of the
   extracted library. This can be done within the `CMakeLists.txt` file or as a
   command-line argument during CMake configuration. Refer to the section on
   [Finding and Linking Against the Library Using find_package()](#finding-and-linking-against-the-library-using-find_package).

This approach allows users to integrate the library into their CMake-based
projects, bypassing the need to build the library from source.
