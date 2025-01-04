# 📦 Exception Handling Library for C

**A robust and thread-safe exception handling library for C**, inspired by modern programming languages. This library provides `try`, `catch`, and `throw` macros for structured error handling in both single-threaded and multi-threaded applications.

---

## 🌟 Features

- **Structured Exception Handling**: Familiar `try`, `catch`, and `throw` constructs.
- **Thread Safety**: Isolates exceptions across threads with thread-local storage.
- **Custom Messages**: Supports `printf`-style formatted messages for exceptions.
- **Nested Exception Propagation**: Handle exceptions across function call levels.
- **Integrated with Modern Tools**: Leverages `.clang-format` and `.clangd` for code consistency and tooling.

---

## 🏗️ Project Structure

```bash
.
├── CMakeLists.txt # Main CMake configuration
├── include/
│    └── exception.h # Library header
├── README.md # Project documentation
├── src/
│    └── exception.c # Library source
└── tests/
      ├── CMakeLists.txt # CMake configuration for tests
      └── exception_tests.c # Unit tests for the library
```

---

## 🚀 Installation

### Prerequisites

- **CMake** (version 3.15 or newer)
- **GCC/Clang** (with C11 support)
- **CMocka** (optional, for testing)

### Build Instructions

1. Clone the Repository:

   ```bash
   git clone https://github.com/mrpriv4te/exception-c.git
   cd exception-c
   ```

2. Create a Build Directory:

   ```bash
   mkdir build && cd build
   ```

3. Configure the Project:

   ```bash
   cmake ..
   ```

4. Build the Library:
   ```bash
   cmake --build .
   ```

### Run the Tests (Optional)

1. Enable Testing During Configuration:

   ```bash
   cmake -DBUILD_TESTING=ON ..
   ```

2. Build and Execute the Tests:
   ```bash
   cmake --build . && ctest
   ```

---

## 🛠️ Integrating the Library into Your Project

### Option 1: Install and Link the Library

1. **Install the Library** (from the build directory):

   ```bash
   cmake --install .
   ```

2. **Update Your Project’s `CMakeLists.txt`**:

   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(YourProject LANGUAGES C)

   # Find and link the exception library

   find_package(ExceptionLibrary CONFIG REQUIRED)

   add_executable(your_project main.c)
   target_link_libraries(your_project PRIVATE exception)
   target_include_directories(your_project PRIVATE ${CMAKE_INSTALL_PREFIX}/include)
   ```

### Option 2: Add the Source Directly

1. **Copy the `src/` and `include/` Directories** into your project directory.

2. **Update Your Project’s `CMakeLists.txt`**:

   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(YourProject LANGUAGES C)

   # Add the exception library

   add_library(exception path/to/src/exception.c)
   target_include_directories(exception PUBLIC path/to/include)

   add_executable(your_project main.c)
   target_link_libraries(your_project PRIVATE exception)
   ```

### Option 3: Add as a Submodule

1. **Add the Submodule**:

   ```bash
   git submodule add https://github.com/mrpriv4te/exception-c.git libs/exception
   ```

2. **Update Your Project’s `CMakeLists.txt`**:

   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(YourProject LANGUAGES C)

   # Add the exception library as a subdirectory

   add_subdirectory(libs/exception)

   add_executable(your_project main.c)
   target_link_libraries(your_project PRIVATE exception)
   ```

---

## 📖 Usage

### 📝 Basic Example

```c
#include "exception.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    try({
        throw(1, "An error occurred: %s", "example message");
    } catch(1) {
        printf("Caught exception: %s\n", exception()->message);
    })

    try({
        int rand_exception_code = rand();
        throw(rand_exception_code, "An other error occurred: %s", "example message");
    } catch(all_exception) {
        printf("Caught exception: %s with rand code %d\n", exception()->message, exception()->code);
    })

    return 0;
}
```

### 🔀 Multithreading Example

```c
#include "exception.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *thread_function(void *arg) {
    int thread_id = *(int *)arg;

    try({
        if (thread_id % 2 == 0) {
            throw(100 + thread_id, "Thread %d exception", thread_id);
        }
    } catch(100 + thread_id) {
        printf("Caught exception in thread %d: %s\n", thread_id, exception()->message);
    })

    pthread_exit(NULL);
}

int main() {
    const int num_threads = 4;
    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
```

---

## ✅ Testing

The project includes unit tests written with [CMocka](https://cmocka.org/) that validate:

- **Basic exception handling**
- **Nested function call propagation**
- **Multithreading with caught and uncaught exceptions**
- **Thread safety across multiple scenarios**

### Run the Tests

1. Enable Testing in CMake:

   ```bash
   cmake -DBUILD_TESTING=ON ..
   ```

2. Build and Execute Tests:

   ```bash
   cmake --build . && ctest
   ```

Example output:

```
...
test 1
    Start 1: ExceptionTests

1: Test command: /path/to/exception-c/build/tests/exception_tests
1: Working Directory: /path/to/exception-c/build/tests
1: Test timeout computed to be: 10000000
1: [==========] tests: Running 7 test(s).
1: [ RUN      ] test_throw_and_catch
1: [       OK ] test_throw_and_catch
1: [ RUN      ] test_throw_uncaught
1: exception_tests: /path/to/exception-c/tests/exception_tests.c:41: test_throw_uncaught(): Uncaught exception (code 2)
1: [       OK ] test_throw_uncaught
1: [ RUN      ] test_message_update
1: [       OK ] test_message_update
1: [ RUN      ] test_nested_function_call
1: [       OK ] test_nested_function_call
1: [ RUN      ] test_multithreaded_exceptions
1: [       OK ] test_multithreaded_exceptions
1: [ RUN      ] test_multithreaded_exceptions_uncaught
1: exception_tests: Thread 126242628044480: /path/to/exception-c/tests/exception_tests.c:130: thread_function_uncaught(): Thread 0 uncaught exception (code 50)
1: exception_tests: Thread 126242649016000: /path/to/exception-c/tests/exception_tests.c:130: thread_function_uncaught(): Thread 2 uncaught exception (code 52)
1: [       OK ] test_multithreaded_exceptions_uncaught
1: [ RUN      ] test_thread_safety
1: [       OK ] test_thread_safety
1: [==========] tests: 7 test(s) run.
1: [  PASSED  ] 7 test(s).
1/1 Test #1: ExceptionTests ...................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
```

---

## 🛠️ Code Formatting and Tooling

- **.clang-format**: Ensures consistent code style across the project. Based on LLVM style.
- **.clangd**: Provides IDE tooling for code navigation, autocompletion, and diagnostics.

### Apply Formatting

To format the source files:

```bash
clang-format -i src/*.c include/*.h
```

---

## 🤝 Contributing

Contributions are welcome! To get started:

1. **Fork the Repository**:

   ```bash
   git fork https://github.com/mrpriv4te/exception-c.git
   ```

2. **Create a Branch**:

   ```bash
   git checkout -b feature/your-feature
   ```

3. **Commit Your Changes**:

   ```bash
   git commit -m "Add your feature"
   ```

4. **Push the Branch**:

   ```bash
   git push origin feature/your-feature
   ```

5. **Submit a Pull Request**.

---

## 📜 License

This project is licensed under the [MIT License](LICENSE).

---

## 🙏 Acknowledgments

- Inspired by exception handling mechanisms in C++ and Python.
- Built with thread safety and modern C practices in mind.

---

Happy coding! 🎉
