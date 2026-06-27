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
- **Arrays** are declared with `new array` and stored as a contiguous block of
  zero-initialized integers in `.data`. Elements are accessed with
  `get element` and `set element`. Array indices are **1-based** (the first
  element is index `1`). The index operand is a **variable**, not a literal.
  Out-of-bounds access at run time terminates the program with exit code `1`.
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
- **Compile-time directives** (`#macro`, `#if` / `#done`, and `#compileTimeInfo`,
  `#compileTimeWarning`, `#compileTimeError`, `#compileTimeDebug`) are handled
  entirely during compilation. They emit no assembly.
- **Marks and jumps** (`mark`, `go to`) let the main program jump to named
  positions. Together with `if` / `done`, they are enough to build loops
  manually.
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

Compiling `test.txt` produces three files next to it:

- `test.S` — generated assembly
- `test.o` — object file
- `test`   — the final executable

Run it with:

```bash
./test
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
| `new array` | `new array A with N elements` | Declare array `A` with `N` integer slots, all initialized to `0`. |
| `get element` | `get element I from array A and put into X` | Copy array slot `I` into variable `X`. |
| `set element` | `set element I from array A to be X` | Copy variable `X` into array slot `I`. |
| `set` | `set X to be N` | Assign integer literal `N` (or a macro name) to `X`. |
| `read` | `read X` | Read an integer from standard input into `X`. |
| `print` | `print X` | Print `X` to standard output as a decimal number. |
| `printString` | `printString message…` | Print a plain text message to standard output (no prefix or trailing newline). |
| `newline` | `newline` | Print a single newline character. |
| `add` | `add Y into X` | `X = X + Y`. |
| `subtract` | `subtract Y from X` | `X = X - Y`. |
| `multiply` | `multiply X by Y` | `X = X * Y`. |
| `divide` | `divide X by Y` | `X = X / Y` (integer division). |
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
| `#macro` | `#macro NAME is N` | Define a compile-time integer constant `NAME` with numeric value `N`. |
| `#if` | `#if A equals to B then do` | Begin a compile-time conditional block. Both operands must be macro names. |
| `#done` | `#done` | Close the most recently opened `#if` block. |
| `#compileTimeInfo` | `#compileTimeInfo message…` | Print a compile-time info line to standard output. Emits no assembly. |
| `#compileTimeWarning` | `#compileTimeWarning message…` | Print a compile-time warning line to standard error. Emits no assembly. |
| `#compileTimeError` | `#compileTimeError message…` | Print a compile-time error line to standard error. Emits no assembly. |
| `#compileTimeDebug` | `#compileTimeDebug message…` | Print a compile-time debug line to standard output. Emits no assembly. |
| `mark` | `mark NAME` | Define a named jump target in the main program. |
| `go to` | `go to NAME` | Jump unconditionally to a previously defined mark. |

### Variables

Declare a variable before using it:

```
new counter
set counter to be 10
```

Variables are global and live for the whole program. Re-declaring an existing
variable, or using one that has not been declared, is a compile error. A
variable cannot share a name with a function or an array, and vice versa.

### Arrays

Declare an array before using it:

```
new array nums with 5 elements
new i
new x

set i to be 1
set x to be 42
set element i from array nums to be x
get element i from array nums and put into x
```

Rules:

- Array names follow the same naming rules as variables and **cannot** share a
  name with a variable, function, or another array.
- The size (`N` in `with N elements`) must be a numeric literal. Array size is
  fixed at compile time; arrays cannot grow at run time.
- Indices are **1-based**: the first slot is index `1`, the last slot is index
  `N`. Index `0` or any index greater than `N` is out of bounds.
- The index (`I`) must be an existing **variable**, not a numeric literal.
- The value (`X`) in `get element` / `set element` must be an existing variable.
- Out-of-bounds access is checked at **run time**. The program exits with code
  `1` if an index is invalid.
- Arrays are global, like variables. Function bodies and the main program share
  the same arrays.

**Loop over an array** (with `mark` / `go to`):

```
new array data with 3 elements
new i
new x

set i to be 1
set x to be 100
set element i from array data to be x

set i to be 2
set x to be 200
set element i from array data to be x

set i to be 1

mark eachElement
if i equals to 4 then do
    go to done
done

get element i from array data and put into x
print x
newline

add one into i
go to eachElement

mark done
```

Because `if` only supports `equals to`, a counter variable (`i`) is the usual
way to walk array indices in a loop.

### Arithmetic

All arithmetic operates on two existing variables. Each instruction uses plain
English word order — read it aloud and it matches the operation:

```
new total
new amount
set total to be 100
set amount to be 25
add amount into total       # total = 125
subtract amount from total  # total = 100
multiply total by amount    # total = 2500
divide total by amount      # total = 100
```

| Instruction | Form | Variable updated |
| --- | --- | --- |
| `add` | `add Y into X` | `X` (after `into`) |
| `subtract` | `subtract Y from X` | `X` (after `from`) |
| `multiply` | `multiply X by Y` | `X` (first operand) |
| `divide` | `divide X by Y` | `X` (first operand / dividend) |

Both operands must already exist. A typo in either variable name is a compile
error.

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
set left to be 5
set right to be 5

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
set left to be 3
set right to be 4

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

### Marks and jumps

`mark` and `go to` provide unstructured control flow in the **main program
only**. They compile to an assembly label and an unconditional `jmp`.

```
mark loop
printString Hello
newline
go to loop
```

Rules:

- Marks may only appear in the main program body, not inside functions.
- `go to` may only appear in the main program body as well.
- Each mark name must be unique across the whole source file.
- A mark must be defined **before** any `go to` that targets it (the compiler
  reads the file in a single pass).
- A mark name cannot be reused and should not match a function name (both become
  assembly labels).

Together with `if` / `else do` / `done`, marks and jumps are enough to build
loops manually, the same way `while` and `for` are usually lowered to labels,
branches, and backward jumps in other languages.

**Infinite loop:**

```
mark loop
printString tick
newline
go to loop
```

**While-style loop** (`keep running while X equals Y`):

```
new X
new Y
new one
set X to be 0
set Y to be 5
set one to be 1

mark whileLoop
if X equals to Y then do
    go to whileEnd
done
print X
add one into X
go to whileLoop
mark whileEnd
```

Because `if` only supports `equals to`, richer comparisons still need workarounds
until more operators are added.

### Macros and compile-time conditionals

Macros are integer constants resolved while the compiler is running. Define
them with `#macro`:

```
#macro TRUE is 1
#macro FALSE is 0
#macro LIMIT is 10
```

Rules:

- Macro names follow the same naming rules as variables and functions.
- A macro value must be a numeric literal.
- Macros can be used anywhere a literal is accepted in `set X to be N` — the
  compiler substitutes the numeric value before generating assembly.

Compile-time conditionals use macro names instead of variables:

```
#macro FEATURE is 1
#macro ENABLED is 1

#if FEATURE equals to ENABLED then do
    new enabledFeature
#done
```

If the condition is false, every line until the matching `#done` is skipped
during compilation (no variables are declared, no assembly is emitted). The form
is exactly seven tokens, same shape as runtime `if`: `#if`, left macro,
`equals`, `to`, right macro, `then`, `do`.

Runtime `done` closes runtime `if` blocks. Compile-time `#done` closes `#if`
blocks. Do not mix them.

### Functions

Functions group instructions that can be called multiple times with `execute`.
Define a function with `function NAME does`, put instructions inside, and close
with `fdone`:

```
new x
new y

function calculate does
    add y into x
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
  `print`, `get element`, `set element`, and so on) can appear inside a function
  body, **except** `mark` and `go to`.

### No-op (`nothing`)

The `nothing` instruction emits a single `nop` instruction. It has no operands
and does nothing at run time. It can be useful as a placeholder while writing
code.

```
nothing
```

### Compile-time debugging

The four `#compileTime*` directives print messages **while the compiler is
running**, not when the compiled program executes. They emit no assembly and are
meant to help you find where compilation stopped when something goes wrong.

Write them through your source as checkpoints:

```
#compileTimeInfo checkpoint 1 start
new x
#compileTimeInfo checkpoint 2 ok
set x to be 10
#compileTimeInfo checkpoint 3 ok
```

When compilation fails, every checkpoint printed before the error shows how far
the compiler got. Checkpoints inside function bodies and `if` blocks are
evaluated during the compile pass (the compiler reads every line in order), even
if that code would not run at execution time.

| Instruction | Output stream | Prefix |
| --- | --- | --- |
| `#compileTimeInfo` | standard output | `INFO: CompileTime Info: …` |
| `#compileTimeWarning` | standard error | `WARNING: CompileTime Warning: …` |
| `#compileTimeError` | standard error | `ERROR: CompileTime Error: …` |
| `#compileTimeDebug` | standard output | `DEBUG: CompileTime Debug: …` |

Each instruction requires at least one message word after the keyword, same as
the runtime `info` / `warning` / `error` / `debug` instructions.

**Important:** instruction detection uses keyword substring matching on the
**entire line**. Because `#compileTime*` keywords are checked after many other
keywords, a compile-time message that contains words like `new`, `set`, `add`,
`done`, `if`, or `print` anywhere in the text can be misclassified as a
different instruction. Keep checkpoint messages free of instruction keywords, or
use neutral wording (for example `#compileTimeInfo checkpoint 2 ok` instead of
`#compileTimeInfo checkpoint 2 after new x`).

### Exiting

```
new code
set code to be 0
exit code
```

If no `exit` runs, the program still terminates cleanly with exit code `0`
(the compiler appends a default exit path that jumps to a shared
`RESERVED_exit_BY_LANGUAGE` helper).

## Example program

Copy the program below into a file (for example `test.txt`) to try out macros,
arrays, functions, marks, and conditionals. It prints ten Fibonacci-style sums
and exits with code `0`:

```
#macro ITERATIONS is 10

new tmp1
new tmp2
new counter
new finishValue
new sum
new i
new zero

set tmp1 to be 0
set tmp2 to be 1
set counter to be 0
set finishValue to be ITERATIONS
set sum to be 0
set i to be 1
set zero to be 0

function finish does
    exit zero
fdone

new array fibonacci with 2 elements
set element i from array fibonacci to be tmp1
set i to be 2
set element i from array fibonacci to be tmp2

mark loop

if counter equals to finishValue then do
    execute finish
done

set i to be 1
get element i from array fibonacci and put into tmp1
set i to be 2
get element i from array fibonacci and put into tmp2
get element i from array fibonacci and put into sum

add tmp1 into sum
add tmp2 into sum

get element i from array fibonacci and put into tmp2
set i to be 1
set element i from array fibonacci to be tmp2
set i to be 2
set element i from array fibonacci to be sum

print sum
newline

set i to be 1

add i into counter
go to loop
```

```bash
./compiler test.txt
./test
```

Sample output:

```
2
5
12
29
70
169
408
985
2378
5741
```

A minimal program that only exits with a code (save as `exitDemo.txt`):

```
#macro exitCode is 67

new code
set code to be exitCode

exit code
```

```bash
./compiler exitDemo.txt
./exitDemo
echo $?    # prints 67
```

### More complete example

A fuller program combining macros, functions, conditionals, and I/O:

```
#macro MAX is 100

new x
new y

function calculate does
    new temp
    set temp to be MAX
    add y into x
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

## Notes and limitations

This is a teaching project, so the language is intentionally minimal and has a
few sharp edges worth knowing:

- **Variable existence is checked by exact name.** Names are registered when
  you declare them with `new`; the compiler tracks them in a dedicated set
  rather than searching the generated `.data` section as text. Short names that
  are substrings of other identifiers (for example `line` vs `newline`) no
  longer cause false "already exists" or "does not exist" errors.
- **Instruction detection is keyword-substring-based.** Avoid variable and
  function names that contain instruction keywords (`new`, `new array`, `set`,
  `set element`, `get element`, `add`, `into`, `from`, `by`, `if`, `read`,
  `printString`, `function`, `execute`, `fdone`, `nothing`, `mark`, `go to`,
  `#macro`, `#compileTimeInfo`, `info`, `warning`, `error`, `debug`, etc.).
  The compiler checks longer keywords such as `new array`, `get element`,
  `set element`, and `#compileTimeInfo` before shorter ones like `new`, `set`,
  `get`, and `info` that they contain. The same rule applies to text in
  `#compileTime*` messages — see [Compile-time debugging](#compile-time-debugging).
- **Arrays have a fixed compile-time size.** There is no heap, dynamic growth,
  or resize. Total memory (variables plus all array slots) is bounded when the
  program is compiled.
- **Array indices are 1-based variables.** You cannot write a literal index
  directly in `get element` or `set element`; use a variable (for example
  `set i to be 1` first).
- **Functions have no parameters or locals.** All variables are global. A `new`
  inside a function creates another global variable, not a local one.
- **`read` uses a single shared buffer.** Reading multiple values from a pipe
  in one go can consume more than one number at once; interactive input (one
  number per line) is the most predictable.
- **No numeric validation.** Non-numeric input parses as `0` or stops at the
  first non-digit; very large values can overflow without warning.
- **Only `equals to` is supported** as a comparison in runtime `if` and
  compile-time `#if`.
- **`if` lines must be exactly** `if X equals to Y then do` and may contain at
  most one `else do` block.
- **`mark` and `go to` are main-program only.** They cannot appear inside
  function bodies. Jumping into or out of a function would break the call/return
  stack, so the compiler rejects them there.
- **Marks must be defined before use.** There is no forward-reference pass; a
  `go to` target must already have been seen earlier in the source file.
- **`exit` requires a variable or macro name**, not a bare numeric literal. Use
  `set code to be 0` and then `exit code`.
- **Unclosed blocks are compile errors.** An `if` without a matching `done`, a
  `#if` without a matching `#done`, or a `function` without a matching `fdone`,
  is rejected after the full source file has been read.
