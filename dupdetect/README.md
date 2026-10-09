# Hashing and Duplicate Detection

A C++17 command-line tool that finds duplicate files by comparing their
contents, not their names. It scans a folder, hashes candidate files with
FNV-1a 64-bit, and confirms every match with a byte-by-byte comparison before
reporting it.

Standard library only (`<filesystem>`, `<fstream>`): no external dependencies.

## Features

- Binary-mode, chunked file reading (constant memory, safe for large files)
- FNV-1a 64-bit hashing in a separate, documented module
- Three-stage detection: size grouping, hash grouping, byte-by-byte confirmation
- Recursive or top-level-only directory scanning
- Readable grouped report with reclaimable-space summary
- Clear errors for missing paths, non-directories, unreadable files and folders
- 54 automated tests with a built-in test runner (no test library needed)

## Project layout

```
src/
  main.cpp              thin entry point (argv -> cli)
  cli.h/.cpp            argument parsing, exit codes, top-level flow
  scanner.h/.cpp        directory scanning
  duplicate_finder.h/.cpp  size -> hash -> byte-compare pipeline
  file_compare.h/.cpp   byte-by-byte comparison
  hasher.h/.cpp         hash a file on disk (streaming)
  hash.h/.cpp           FNV-1a 64-bit algorithm
  file_reader.h/.cpp    safe binary file reading
  file_error.h          per-file error record
  report.h/.cpp         text report and size formatting
tests/
  test_framework.h      minimal TEST_CASE / CHECK / CHECK_EQ framework
  test_helpers.h        temp-folder and file-writing helpers
  test_main.cpp         runs all tests
  test_hash.cpp         hashing unit tests
  test_file_io.cpp      file reading and file hashing tests
  test_duplicates.cpp   comparison and duplicate detection tests
  test_scanner.cpp      directory scanning tests
  test_integration.cpp  end-to-end workflow and CLI tests
```

All code is in the `dupdetect` namespace and the module has its own Makefile,
so it can live in a sub-folder of a larger repository without clashing with
other code.

## Building

Requirements: a C++17 compiler (GCC 9 or newer) and `make`.

### Windows (MSYS2 UCRT64)

Open the **MSYS2 UCRT64** terminal (or a VS Code terminal using that shell) and
install the tools once:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc make
```

Then, from the project folder:

```bash
make
make test
make clean
```

Without `make`, compile directly:

```bash
mkdir -p build
g++ -std=c++17 -Wall -Wextra -O2 -Isrc src/*.cpp -o build/dupdetect
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -Itests $(ls src/*.cpp | grep -v main.cpp) tests/*.cpp -o build/run_tests
```

The VS Code *Code Runner* extension compiles only the open file, so it cannot
build this multi-file project; use the terminal commands above.

### Linux / macOS

The same `make` and `make test` commands work.

## Usage

```
dupdetect <directory> [options]

Options:
  --no-recursive   only scan the top level of <directory>
  --include-empty  also report empty (0-byte) files as duplicates
  -h, --help       show help
```

Example:

```bash
./build/dupdetect "C:/Users/me/Pictures"
```

```
Scanning: C:/Users/me/Pictures (recursive)

Found 1 group(s) of duplicate files.

Group 1: 3 identical files, 2.4 MiB each (FNV-1a 64: 0e9154a5da464b85)
  [1] C:/Users/me/Pictures/backup/IMG_0042.jpg
  [2] C:/Users/me/Pictures/IMG_0042.jpg
  [3] C:/Users/me/Pictures/old/IMG_0042 (1).jpg

Summary
  Files checked       : 128
  Empty files skipped : 2
  Files with errors   : 0
  Duplicate groups    : 1
  Redundant copies    : 2
  Reclaimable space   : 4.8 MiB
```

Warnings (for example, a file that could not be opened) go to standard error;
the report goes to standard output, so `dupdetect dir > report.txt` keeps the
report clean.

Exit codes: `0` scan finished (with or without duplicates), `1` invalid
arguments, `2` the directory could not be scanned.

The tool only reports duplicates. It never deletes or modifies files.

## How it works

1. **Scan**: collect regular files under the directory. Symbolic links are
   skipped (so a file is not counted twice and link loops cannot occur).
   Sub-folders that cannot be opened are reported and skipped.
2. **Group by size**: files with a unique size cannot have a duplicate and
   are never read. This is the cheapest filter.
3. **Group by hash**: files sharing a size are streamed through FNV-1a 64.
4. **Confirm**: files sharing a hash are compared byte by byte. Only files
   that are truly identical are reported.

## Design decisions

**Why FNV-1a and not SHA-256?** FNV-1a is a few lines of code, fast, and has
no dependencies, which keeps the module easy to read. It is **not** a
cryptographic hash: collisions are possible and can be crafted on purpose.
That is acceptable here because the hash is only a filter; step 4 makes the
final decision, so a collision can cost an extra comparison but can never
produce a false duplicate. If the tool ever had to trust hashes alone (for
example, comparing against hashes stored from a previous run, where the old
file is no longer available to compare), a cryptographic hash such as SHA-256
would be needed, e.g. from OpenSSL (`pacman -S mingw-w64-ucrt-x86_64-openssl`).

**Why size first?** Getting a file size costs one metadata lookup; hashing
reads the whole file. Most files in a typical folder have unique sizes, so
this avoids most of the reading.

**Why collect errors instead of stopping?** One locked or deleted file should
not prevent checking thousands of others. Errors are returned as `FileError`
records and printed as warnings.

**Why an injectable hash function?** `FinderOptions::hashFunction` lets the
tests replace the real hash with one that always collides, which is the only
practical way to prove the byte-comparison safeguard works.

## Testing

```bash
make test
```

The runner prints `[PASS]`/`[FAIL]` per test and exits with code 0 only if
all tests pass. Tests create their files under the system temp folder
(`<temp>/dupdetect_tests/`), so they never touch project files.

Covered: FNV-1a reference vectors, empty/identical/different content, binary
bytes (including `\0` and `\r\n`), chunked reading, missing files,
directories passed as files, byte comparison across multiple chunks, forced
hash collisions, repeated paths, empty-file handling, recursive and flat
scanning, symlink skipping, CLI parsing, exit codes and full report output.

Not covered automatically: permission-denied files and folders (behaviour
depends on the OS and on running as administrator/root), and files changing
while being scanned. The unreadable-folder path was checked manually on Linux
as a non-root user.

## Known limitations

- Hard links to the same file are reported as duplicates (they share content).
- Paths with characters outside the system code page may display incorrectly
  on Windows, since paths are printed with `path::string()`.
- Files are compared against the first file of each candidate group, so a very
  large number of same-hash, different-content files would be slow; with
  FNV-1a 64 this is not expected in practice.
