# GEMINI.md - LISP/c (lispsy)

## Project Overview
**LISP/c** (pronounced "lispsy") is a powerful macrolanguage and translator that brings the expressiveness of **LISP** to the performance of **C** and **C++**. It allows developers to write code in a Lisp-like syntax which is then translated into standard C or C++ source code. The project is specifically designed to handle high-performance computing tasks, featuring built-in support for **CUDA**, **MPI**, and **Pthreads**.

The core engine is implemented in **Common Lisp** (primarily tested with **CLISP**) and leverages Lisp's macro system to provide advanced code generation capabilities that far exceed the standard C preprocessor.

## Key Features
- **Lisp-to-C Translation:** Write C logic using S-expressions.
- **Advanced Macros:** Use `lispmacro` for Lisp-time code generation and `template` for C-style code templating.
- **HPC Support:** Dedicated syntax and synonyms for CUDA kernels, MPI communication, and Pthreads multithreading.
- **Flexible Syntax:** Support for camelCase generation, automatic pointer handling, and prefix arithmetic.
- **C++ Integration:** Support for C++ features like classes, namespaces, and stream operators (`<<+`, `>>+`).

## Building and Running

### Prerequisites
- **CLISP:** The project currently requires CLISP to run the translation engine.
- **GCC:** A C compiler (like GCC) is needed to compile the generated C code.
- **CUDA/MPI (Optional):** Required if you intend to compile and run CUDA or MPI examples.

### Usage
1.  **Load the Engine:**
    Start `clisp` and load the main script:
    ```lisp
    (load "c.lisp")
    ```

2.  **Translate a `.cl` file to `.c`:**
    ```lisp
    (c-cl-file "source.cl" "dest.c")
    ```

3.  **Compile and Run:**
    ```lisp
    (compile-and-run-cl-file "file.cl")
    ```

4.  **Preview in REPL:**
    To see the generated C code directly in the terminal:
    ```lisp
    (cwf "filename.cl")
    ```

## Development Conventions

### File Extensions
- **`.cl`**: LISP/c source files.
- **`.c` / `.cpp`**: Generated output files.
- **`c.lisp`**: The main engine source.

### Coding Style
- **Identifiers:** Use kebab-case (e.g., `my-function-name`). The translator automatically converts these to snake_case (`my_function_name`) in C.
- **Constants:** Use a `!` prefix for uppercase constants (e.g., `!nthreads` -> `NTHREADS`).
- **Function Calls:** Use the `@` prefix for concise function calls (e.g., `(@printf "hello")`).
- **Pointer Types:** Use `(pt name)` or `(typ* type)` for pointer declarations.
- **Memory Access:** Use `[]` notation for array indexing (e.g., `([]my-array i)`).

### Macros and Templates
- **`template`**: Used for generating repetitive C code blocks with different types or variables.
- **`lispmacro`**: Allows executing arbitrary Lisp code during the translation phase to generate C structures.

## Key Files
- `c.lisp`: The heart of the project, containing the translator logic and macro definitions.
- `README.md`: Comprehensive documentation with numerous examples.
- `test.cl`: A sample file demonstrating linked list implementation and basic usage.
- `cuda.cl`: Example showing CUDA kernel generation.
- `multi.cl`: Example of Pthreads multithreading.
- `myls.cl`: A complex example implementing a version of the `ls` command.
