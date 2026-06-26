# compiler-on-cpp

A small educational compiler written in C++ that translates a tiny custom
language into **x86-64 GNU Assembler (GAS)** source, then assembles and links it
into a native Linux executable.

## Overview

The compiler reads a source file line by line, turns each line into a chunk of
assembly, and writes the result to a `.S` file. It then invokes the GNU
toolchain to produce a runnable binary:

```
source.txt  -->  compiler  -->  source.S  -->  as  -->  source.o  -->  ld  -->  source
```

- Each program variable becomes a global in the `.data` section (a `.quad` in
  64-bit mode, a `.long` in 32-bit mode).
- Integer printing is handled by a generated `itoa` routine; reading integers
  from standard input is handled by a generated `atoi` routine. These helpers
  are only emitted when the program actually uses `print` / `read`.
- Labeled text output (`info`, `warning`, `error`, `debug`) and plain text output
  (`printString`) use a generated `get_string_length` routine. It is emitted when
  the program uses any of those instructions or `print`. `warning` and `error`
  write to standard error; `info`, `debug`, and `printString` write to standard
  output.
- The program entry point is `_start`, and execution ends with the Linux
  `exit` syscall.

The language works exclusively with **signed integers**.

## Requirements

- Linux on **x86-64**
- A C++ compiler with **C++20** support (uses `std::format`), e.g. `g++` 13+
- **GNU binutils**: `as` (assembler) and `ld` (linker) must be on your `PATH`

## Building the compiler

```bash
g++ -std=c++20 main.cpp -o compiler
```

## Usage

```bash
./compiler [flags] <source-file>
```

Compiling `exampleCode.txt` produces three files next to it:

- `exampleCode.S` — generated assembly
- `exampleCode.o` — object file
- `exampleCode`   — the final executable

Run it with:

```bash
./exampleCode
```

### Compiler flags

| Flag | Description |
| --- | --- |
| `--64bits` | Generate 64-bit code (this is the **default**). |
| `--32bits` | Generate 32-bit-width variables (`.long`) and 32-bit arithmetic. |
| `--clearObjectFiles` | Delete `*.o` files after a successful build. |
| `--clearAssemblyFiles` | Delete `*.S` files after a successful build. |

The first non-flag argument is treated as the source file. If compilation
fails, the generated `.S` for the source is removed automatically.

## Language syntax

One instruction per line. Tokens are separated by whitespace. Blank lines are
ignored. There are no comments.

| Instruction | Form | Meaning |
| --- | --- | --- |
| `new` | `new X` | Declare integer variable `X`, initialized to `0`. |
| `set` | `set X N` | Assign integer literal `N` to `X`. |
| `read` | `read X` | Read an integer from standard input into `X`. |
| `print` | `print X` | Print `X` to standard output as a decimal number. |
| `printString` | `printString message…` | Print a plain text message to standard output (no prefix or trailing newline). |
| `newline` | `newline` | Print a single newline character. |
| `add` | `add X Y` | `X = X + Y`. |
| `subtract` | `subtract X Y` | `X = X - Y`. |
| `multiply` | `multiply X Y` | `X = X * Y`. |
| `divide` | `divide X Y` | `X = X / Y` (integer division). |
| `if` | `if X equals Y then` | Run the block until `else`/`done` only when `X == Y`. The `then` keyword is required. |
| `else` | `else` | Begin the block that runs only when the matching `if` condition was false. Optional. |
| `done` | `done` | Close the most recently opened `if` block. |
| `exit` | `exit X` | Terminate the program with exit code `X`. |
| `info` | `info message…` | Print an informational line to standard output: `INFO: message…`. |
| `warning` | `warning message…` | Print a warning line to standard error: `WARNING: message…`. |
| `error` | `error message…` | Print an error line to standard error: `ERROR: message…`. |
| `debug` | `debug message…` | Print a debug line to standard output: `DEBUG: message…`. |

### Variables

Declare a variable before using it:

```
new counter
set counter 10
```

Variables are global and live for the whole program. Re-declaring an existing
variable, or using one that has not been declared, is a compile error.

### Arithmetic

All arithmetic operates on two existing variables and stores the result back
into the first operand:

```
new total
new amount
set total 100
set amount 25
add total amount        # total = 125
subtract total amount   # total = 100
multiply total amount   # total = 2500
```

### Input and output

```
new value
read value      # type a number and press Enter
print value     # prints it back
newline
```

### Plain text output

The `printString` instruction prints a raw text message to standard output. It
takes one or more words after the keyword; those words are written exactly as
given, with spaces between them. Unlike `info` / `warning` / `error` / `debug`,
there is no level prefix and no automatic newline at the end.

```
printString Hello World!
newline
```

Running the program above prints:

```
Hello World!
```

Multi-word messages are supported:

```
printString value is ready
newline
```

prints `value is ready` followed by a newline. At least one message word is
required; a line with only `printString` is a compile error.

### Logging messages

The `info`, `warning`, `error`, and `debug` instructions print a labeled line of
text. Each instruction takes one or more words after the keyword; those words
become the message body. The compiler adds the level prefix and a trailing
newline automatically.

`info` and `debug` write to **standard output**. `warning` and `error` write to
**standard error**, so they can be separated from normal program output when
redirecting streams (for example `./program > out.txt` keeps warnings and errors
on the terminal).

```
info This is an info!
warning This is a warning!
error This is an error!
debug This is a debug!
```

Running the program above prints:

```
INFO: This is an info!
DEBUG: This is a debug!
```

to standard output, and:

```
WARNING: This is a warning!
ERROR: This is an error!
```

to standard error.

Multi-word messages are written as a single line:

```
info value is ready
```

prints `INFO: value is ready`. At least one message word is required; a line
with only the keyword (for example `info` alone) is a compile error.

### Conditionals

A conditional starts with `if X equals Y then` and ends with `done`. The `then`
keyword is **required** at the end of the `if` line. The block runs only when the
two variables are equal; otherwise execution jumps past `done`. Conditionals can
be **nested**.

```
new left
new right
set left 5
set right 5

if left equals right then
  print left
  newline
done
```

#### `else`

An optional `else` block runs only when the `if` condition is false. It goes
between the `if` and its matching `done`:

```
new left
new right
set left 3
set right 4

if left equals right then
  print left
  newline
else
  print right
  newline
done
```

When `left == right`, only the `then` block runs; otherwise only the `else`
block runs. Each `if` may have at most one `else`.

### Exiting

```
new code
set code 0
exit code
```

If no `exit` runs, the program still terminates cleanly with exit code `0`
(a default exit is appended automatically when no `exit` instruction is
present in the source).

## Example program

```
new x
read x

new y
read y

add x y

print x
newline

new code
set code 0
exit code
```

Build and run:

```bash
./compiler exampleCode.txt
```

## Notes and limitations

This is a teaching project, so the language is intentionally minimal and has a
few sharp edges worth knowing:

- **Variable name matching is substring-based.** A name is considered to
  "exist" if it appears anywhere in the generated data section, so short names
  that are substrings of others (or of keywords like `newline`) can collide.
  Prefer distinct, multi-character variable names.
- **Instruction detection is keyword-substring-based.** Avoid variable names
  that contain instruction keywords (`new`, `set`, `add`, `if`, `read`,
  `printString`, `info`, `warning`, `error`, `debug`, etc.). The compiler
  checks longer keywords such as `printString` before shorter ones like `print`
  that they contain.
- **`read` uses a single shared buffer.** Reading multiple values from a pipe
  in one go can consume more than one number at once; interactive input (one
  number per line) is the most predictable.
- **No numeric validation.** Non-numeric input parses as `0` or stops at the
  first non-digit; very large values can overflow without warning.
- **Only `equals` is supported** as a comparison in `if`.
- **`if` lines must end with `then`** and may contain at most one `else` block.
