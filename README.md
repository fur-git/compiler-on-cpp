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
- Integer printing is handled by a generated `RESERVED_itoa_BY_LANGUAGE` routine;
  reading integers from standard input is handled by a generated
  `RESERVED_atoi_BY_LANGUAGE` routine. These helpers are only emitted when the
  program actually uses `print` / `read`.
- Labeled text output (`info`, `warning`, `error`, `debug`) and plain text output
  (`printString`) use a generated `RESERVED_get_string_BY_length_LANGUAGE`
  routine. It is emitted when the program uses any of those instructions or
  `print`. `warning` and `error` write to standard error; `info`, `debug`, and
  `printString` write to standard output.
- User-defined **functions** are emitted into a separate `.text` section and
  invoked with `execute`. The main program body and function bodies share the
  same global variables.
- **Compile-time messages** (`compileTimeInfo`, `compileTimeWarning`,
  `compileTimeError`, `compileTimeDebug`) print diagnostic output while the
  compiler is running. They emit no assembly and are useful for finding where
  compilation stopped when something goes wrong.
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
| `if` | `if X equals to Y then do` | Run the block until `else do`/`done` only when `X == Y`. All keywords are required. |
| `else` | `else do` | Begin the block that runs only when the matching `if` condition was false. Optional. |
| `done` | `done` | Close the most recently opened `if` block. |
| `exit` | `exit X` | Terminate the program with exit code `X`. |
| `function` | `function NAME does` | Begin a function definition named `NAME`. |
| `fdone` | `fdone` | End the current function definition. |
| `execute` | `execute NAME` | Call the function named `NAME`. |
| `nothing` | `nothing` | Emit a no-op (`nop`) at run time. |
| `info` | `info message…` | Print an informational line to standard output: `INFO: message…`. |
| `warning` | `warning message…` | Print a warning line to standard error: `WARNING: message…`. |
| `error` | `error message…` | Print an error line to standard error: `ERROR: message…`. |
| `debug` | `debug message…` | Print a debug line to standard output: `DEBUG: message…`. |
| `compileTimeInfo` | `compileTimeInfo message…` | Print a compile-time info line to standard output. Emits no assembly. |
| `compileTimeWarning` | `compileTimeWarning message…` | Print a compile-time warning line to standard error. Emits no assembly. |
| `compileTimeError` | `compileTimeError message…` | Print a compile-time error line to standard error. Emits no assembly. |
| `compileTimeDebug` | `compileTimeDebug message…` | Print a compile-time debug line to standard output. Emits no assembly. |

### Variables

Declare a variable before using it:

```
new counter
set counter 10
```

Variables are global and live for the whole program. Re-declaring an existing
variable, or using one that has not been declared, is a compile error. A
variable cannot share a name with a function, and vice versa.

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

A conditional starts with `if X equals to Y then do` and ends with `done`. Every
keyword on the `if` line is **required** — the form is exactly seven tokens:
`if`, left operand, `equals`, `to`, right operand, `then`, `do`. The block runs
only when the two variables are equal; otherwise execution jumps past `done`.
Conditionals can be **nested**.

```
new left
new right
set left 5
set right 5

if left equals to right then do
  print left
  newline
done
```

#### `else do`

An optional `else do` block runs only when the `if` condition is false. It goes
between the `if` and its matching `done`:

```
new left
new right
set left 3
set right 4

if left equals to right then do
  print left
  newline
else do
  print right
  newline
done
```

When `left == right`, only the `then` block runs; otherwise only the `else`
block runs. Each `if` may have at most one `else do`.

### Functions

Functions group instructions that can be called multiple times with `execute`.
Define a function with `function NAME does`, put instructions inside, and close
with `fdone`:

```
new x
new y

function calculate does
    add x y
fdone

read x
read y
execute calculate
print x
newline
```

Rules:

- Function names follow the same naming rules as variables and **cannot** share a
  name with a variable.
- Nested functions are not allowed — you cannot define a function inside another
  function.
- A function must be closed with `fdone`; leaving it open is a compile error.
- Functions share global variables with the main program. There are no local
  variables or parameters.
- `execute NAME` emits a `call` to the function. The function must already be
  defined earlier in the source file.
- Any instruction that can appear in the main program (including `if`, `exit`,
  `print`, and so on) can appear inside a function body.

### No-op (`nothing`)

The `nothing` instruction emits a single `nop` instruction. It has no operands
and does nothing at run time. It can be useful as a placeholder while writing
code.

```
nothing
```

### Compile-time debugging

The four `compileTime*` instructions print messages **while the compiler is
running**, not when the compiled program executes. They emit no assembly and are
meant to help you find where compilation stopped when something goes wrong.

Write them through your source as checkpoints:

```
compileTimeInfo checkpoint 1 start
new x
compileTimeInfo checkpoint 2 ok
set x 10
compileTimeInfo checkpoint 3 ok
```

When compilation fails, every checkpoint printed before the error shows how far
the compiler got. Checkpoints inside function bodies and `if` blocks are
evaluated during the compile pass (the compiler reads every line in order), even
if that code would not run at execution time.

| Instruction | Output stream | Prefix |
| --- | --- | --- |
| `compileTimeInfo` | standard output | `INFO: CompileTime Info: …` |
| `compileTimeWarning` | standard error | `WARNING: CompileTime Warning: …` |
| `compileTimeError` | standard error | `ERROR: CompileTime Error: …` |
| `compileTimeDebug` | standard output | `DEBUG: CompileTime Debug: …` |

Each instruction requires at least one message word after the keyword, same as
the runtime `info` / `warning` / `error` / `debug` instructions.

**Important:** instruction detection uses keyword substring matching on the
**entire line**. Because `compileTime*` keywords are checked after many other
keywords, a compile-time message that contains words like `new`, `set`, `add`,
`done`, `if`, or `print` anywhere in the text can be misclassified as a
different instruction. Keep checkpoint messages free of instruction keywords, or
use neutral wording (for example `compileTimeInfo checkpoint 2 ok` instead of
`compileTimeInfo checkpoint 2 after new x`).

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

`exampleCode.txt` demonstrates variables, a function, conditionals, and I/O:

```
new x
new y

function calculate does
    new temp
    set temp 100
    add x y
    if x equals to temp then do
        printString The result equals to 100!
        newline
        exit temp
    done
fdone

read x
read y

execute calculate
print x
newline
```

Build and run:

```bash
./compiler exampleCode.txt
./exampleCode
```

## Notes and limitations

This is a teaching project, so the language is intentionally minimal and has a
few sharp edges worth knowing:

- **Variable name matching is substring-based.** A name is considered to
  "exist" if it appears anywhere in the generated data section, so short names
  that are substrings of others (or of keywords like `newline`) can collide.
  Prefer distinct, multi-character variable names.
- **Instruction detection is keyword-substring-based.** Avoid variable and
  function names that contain instruction keywords (`new`, `set`, `add`, `if`,
  `read`, `printString`, `function`, `execute`, `fdone`, `nothing`,
  `compileTimeInfo`, `info`, `warning`, `error`, `debug`, etc.). The compiler
  checks longer keywords such as `printString` and `compileTimeInfo` before
  shorter ones like `print` and `info` that they contain. The same rule applies
  to text in `compileTime*` messages — see [Compile-time debugging](#compile-time-debugging).
- **Arithmetic checks only the first operand.** Instructions like `add X Y`
  verify that `X` exists but not `Y`. A typo in `Y` may pass compilation and
  fail at assembly or link time instead.
- **Functions have no parameters or locals.** All variables are global. A `new`
  inside a function creates another global variable, not a local one.
- **`read` uses a single shared buffer.** Reading multiple values from a pipe
  in one go can consume more than one number at once; interactive input (one
  number per line) is the most predictable.
- **No numeric validation.** Non-numeric input parses as `0` or stops at the
  first non-digit; very large values can overflow without warning.
- **Only `equals to` is supported** as a comparison in `if`.
- **`if` lines must be exactly** `if X equals to Y then do` and may contain at
  most one `else do` block.
- **Unclosed blocks are compile errors.** An `if` without a matching `done`, or
  a `function` without a matching `fdone`, is rejected after the full source
  file has been read.
