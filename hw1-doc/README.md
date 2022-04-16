# Homework 1 - CSE 320 - Spring 2022
#### Professor Eugene Stark

### **Due Date: Friday 02/18/2022 @ 11:59pm**

**Read the entire doc before you start**

## Introduction

In this assignment, you will implement functions for parsing JSON input
and building a data structure to represent its contents and for traversing
the data structure and producing JSON output.
You will use these functions to implement a command-line utility
(called `argo`)
which can validate JSON input and transform JSON input into JSON output
in a "canonical" form.
The goal of this homework is to familiarize yourself with C programming,
with a focus on input/output, bitwise manipulations, and the use of pointers.

For all assignments in this course, you **MUST NOT** put any of the functions
that you write into the `main.c` file.  The file `main.c` **MUST ONLY** contain
`#include`s, local `#define`s and the `main` function (you may of course modify
the `main` function body).  The reason for this restriction has to do with our
use of the Criterion library to test your code.
Beyond this, you may have as many or as few additional `.c` files in the `src`
directory as you wish.  Also, you may declare as many or as few headers as you wish.
Note, however, that header and `.c` files distributed with the assignment base code
often contain a comment at the beginning which states that they are not to be
modified.  **PLEASE** take note of these comments and do not modify any such files,
as they will be replaced by the original versions during grading.

> :scream: Array indexing (**'A[]'**) is not allowed in this assignment. You
> **MUST USE** pointer arithmetic instead. All necessary arrays are declared in
> the `global.h` header file. You **MUST USE** these arrays. **DO NOT** create
> your own arrays. We **WILL** check for this.

> :nerd: Reference for pointers: [https://beej.us/guide/bgc/html/#pointers](https://beej.us/guide/bgc/html/#pointers).

# Getting Started

Fetch base code for `hw1` as described in `hw0`. You can find it at this link:
[https://gitlab02.cs.stonybrook.edu/cse320/hw1](https://gitlab02.cs.stonybrook.edu/cse320/hw1).
**IMPORTANT: 'FETCH', DO NOT 'CLONE'.**

Both repos will probably have a file named `.gitlab-ci.yml` with different contents.
Simply merging these files will cause a merge conflict. To avoid this, we will
merge the repos using a flag so that the `.gitlab-ci.yml` found in the `hw1`
repo will replace the `hw0` version.  To merge, use this command:

```
git merge -m "Merging HW1_CODE" HW1_CODE/master --strategy-option=theirs
```

> :scream: Based on past experience, many students will either ignore the above command or forget
> to use it.  The result will be a **merge conflict**, which will be reported by git.
> Once a merge conflict has been reported, it is essential to correct it before committing
> (or to abort the merge without committing -- use `git merge --abort` and go back and try again),
> because git will have inserted markers into the files involved indicating the locations of the
> conflicts, and if you ignore this and commit anyway, you will end up with corrupted files.
> You should consider it important to read up at an early stage on merge conflicts with git and
> how to resolve them properly.

Here is the structure of the base code:

<pre>
.
├── .gitlab-ci.yml
└── hw1
    ├── .gitignore
    ├── hw1.sublime-project
    ├── include
    │   ├── argo.h
    │   ├── debug.h
    │   └── global.h
    ├── lib
    │   └── argo.a
    ├── Makefile
    ├── rsrc
    │   ├── numbers.json
    │   ├── package-lock.json
    │   └── strings.json
    ├── src
    │   ├── argo.c
    │   ├── const.c
    │   ├── main.c
    │   └── validargs.c
    ├── test_output
    │   └── .git_keep
    └── tests
        ├── basecode_tests.c
        └── rsrc
            └── strings_-c.json
</pre>

- The `.gitlab-ci.yml` file is a file that specifies "continuous integration" testing
to be performed by the GitLab server each time you push a commit.  Usually it will
be configured to check that your code builds and runs, and that any provided unit tests
are passed.  You are free to change this file if you like.

> :scream:  The CI testing is for your own information; it does not directly have
> anything to do with assignment grading or whether your commit has been properly
> pushed to the server.  If some part of the testing fails, you will see the somewhat
> misleading message "commit failed" on the GitLab web interface.
> This does **not** mean that "your attempt to commit has failed" or that "your commit
> didn't get pushed to the server"; the very fact that the testing was triggered at
> all means that you successfully pushed a commit.  Rather, it means that "the CI tests
> performed on a commit that you pushed did not succeed".  The purpose of the tests are
> to alert you to possible problems with your code; if you see that testing has failed
> it is worth investigating why that has happened.  However, the tests can sometimes
> fail for reasons that are not your fault; for example, the entire CI "runner" system
> may fail if someone submits code that fills up the system disk.  You should definitely
> try to understand why the tests have failed if they do, but it is not necessary to be
> overly obsessive about them.

- The `hw1.sublime-project` file is a "project file" for use by the Sublime Text editor.
It is included to try to help Sublime understand the organization of the project so that
it can properly identify errors as you edit your code.

- The `Makefile` is a configuration file for the `make` build utility, which is what
you should use to compile your code.  In brief, `make` or `make all` will compile
anything that needs to be, `make debug` does the same except that it compiles the code
with options suitable for debugging, and `make clean` removes files that resulted from
a previous compilation.  These "targets" can be combined; for example, you would use
`make clean debug` to ensure a complete clean and rebuild of everything for debugging.

- The `include` directory contains C header files (with extension `.h`) that are used
by the code.  Note that these files often contain `DO NOT MODIFY` instructions at the beginning.
You should observe these notices carefully where they appear.

- The `src` directory contains C source files (with extension `.c`).

- The `tests` directory contains C source code (and sometimes headers and other files)
that are used by the Criterion tests.

- The `rsrc` directory contains some samples of data files that you can use for
testing purposes.

 - The `test_output` directory is a scratch directory where the Criterion tests can
put output files.  You should not commit any files in this directory to your
`git` repository.

- The `lib` directory contains a library with binaries for my functions
`argo_read_value()` and `argo_write_value()`.  As discussed below, by commenting out
the stubs for these functions in `argo.c` you can arrange for my versions to be
linked with your code, which may help you to get a jump start on understanding
some things.

## A Note about Program Output

What a program does and does not print is VERY important.
In the UNIX world stringing together programs with piping and scripting is
commonplace. Although combining programs in this way is extremely powerful, it
means that each program must not print extraneous output. For example, you would
expect `ls` to output a list of files in a directory and nothing else.
Similarly, your program must follow the specifications for normal operation.
One part of our grading of this assignment will be to check whether your program
produces EXACTLY the specified output.  If your program produces output that deviates
from the specifications, even in a minor way, or if it produces extraneous output
that was not part of the specifications, it will adversely impact your grade
in a significant way, so pay close attention.

> :scream: Use the debug macro `debug` (described in the 320 reference document in the
> Piazza resources section) for any other program output or messages you many need
> while coding (e.g. debugging output).

# Part 1: Program Operation and Argument Validation

In this part of the assignment, you will write a function to validate the arguments
passed to your program via the command line. Your program will treat arguments
as follows:

- If no flags are provided, you will display the usage and return with an
`EXIT_FAILURE` return code.

- If the `-h` flag is provided, you will display the usage for the program and
  exit with an `EXIT_SUCCESS` return code.

- If the `-v` flag is provided, then the program will read data from standard input
(`stdin`) and validate that it is syntactically correct JSON.  If so, the program
exits with an `EXIT_SUCCESS` return code, otherwise the program exits with an
`EXIT_FAILURE` return code.  In the latter case, the program will print to
standard error (`stderr`) an error message describing the error that was discovered.
No other output is produced.

- If the `-c` flag is provided, then the program performs the same function as
described for `-v`, but after validating the input, the program will also output
to standard output (`stdout`) a "canonicalized" version of the input.
"Canonicalized" means that the output is in a standard form in which possibilities
for variation have been eliminated.  This is described in more detail below.
Unless `-p` has also been specified, then the produced output contains **no whitespace**
(except within strings that contain whitespace characters).

- If the `-p` flag is provided, then the `-c` flag must also have been provided.
In this case, newlines and spaces are used to format the canonicalized output
in a more human-friendly way.  See below for the precise requirements on where
this whitespace must appear.  The `INDENT` is an optional nonnegative integer argument
that specifies the number of additional spaces to be output at the beginning of a line
for each increase in indentation level.  The format of this argument must be
the same as for a nonnegative integer number in the JSON specification.
If `-p` is provided without any `INDENT`, then a default value of 4 is used.

Note that the program reads data from `stdin` and writes transformed data
to `stdout`.  Any other printout, such as diagnostic messages produced by the
program, are written to `stderr`.  If the program runs without error, then it
will exit with the `EXIT_SUCCESS` status code; if any error occurs during the
execution of the program, then it will exit with the `EXIT_FAILURE` status code.

> :nerd: `EXIT_SUCCESS` and `EXIT_FAILURE` are macros defined in `<stdlib.h>` which
> represent success and failure return codes respectively.

> :nerd: `stdin`, `stdout`, and `stderr` are special I/O "streams", defined
> in `<stdio.h>`, which are automatically opened at the start of execution
> for all programs, do not need to be reopened, and (almost always) should not
> be closed.

The usage scenarios for this program are described by the following message,
which is printed by the program when it is invoked without any arguments:

<pre>
USAGE: bin/argo [-h] [-c|-v] [-p|-p INDENT]
   -h       Help: displays this help menu.
   -v       Validate: the program reads from standard input and checks whether
            it is syntactically correct JSON.  If there is any error, then a message
            describing the error is printed to standard error before termination.
            No other output is produced.
   -c       Canonicalize: once the input has been read and validated, it is
            re-emitted to standard output in 'canonical form'.  Unless -p has been
            specified, the canonicalized output contains no whitespace (except within
            strings that contain whitespace characters).
   -p       Pretty-print:  This option is only permissible if -c has also been specified.
            In that case, newlines and spaces are used to format the canonical output
            in a more human-friendly way.  For the precise requirements on where this
            whitespace must appear, see the assignment handout.
            The INDENT is an optional nonnegative integer argument that specifies the
            number of additional spaces to be output at the beginning of a line for each
            for each increase in indentation level.  If no value is specified, then a
            default value of 4 is used.
</pre>

The square brackets indicate that the enclosed argument is optional.
The `-c|-v` means that one of `-c` or `-v` may be specified.
The `-p|-p INDENT` means that `-p` may be specified alone, or with an optional
additional argument `INDENT`.

A valid invocation of the program implies that the following hold about
the command-line arguments:

- All "positional arguments" (`-h`, `-c`, or `-v`) come before any optional arguments
(`-p`).
The optional arguments (well, there is only one) may come in any order after the positional ones.

- If the `-h` flag is provided, it is the first positional argument after
the program name and any other arguments that follow are ignored.

- If the `-h` flag is *not* specified, then exactly one of `-v` or `-c`
must be specified.

- If `-p` is given, then it might or might not be followed by an `INDENT` argument.
  If the `INDENT` argument is present, then it must represent a nonnegative integer
  in the format allowed for integer numbers in the JSON specification.

For example, the following are a subset of the possible valid argument
combinations:

- `$ bin/argo -h ...`
- `$ bin/argo -v`
- `$ bin/argo -c -p`
- `$ bin/argo -c -p 8`

> :scream: The `...` means that all arguments, if any, are to be ignored; e.g.
> the usage `bin/argo -h -x -y BLAHBLAHBLAH -z` is equivalent to `bin/argo -h`.

Some examples of invalid combinations would be:

- `$ bin/argo -p 1 -c`
- `$ bin/argo -v -c`
- `$ bin/argo -v -p 5`
- `$ bin/argo -z 20`

> :scream: You may use only "raw" `argc` and `argv` for argument parsing and
> validation. Using any libraries that parse command line arguments (e.g.
> `getopt`) is prohibited.

> :scream: Any libraries that help you parse strings are prohibited as well
> (`string.h`, `ctype.h`, etc).  The use of `atoi`, `scanf`, `fscanf`, `sscanf`,
> and similar functions is likewise prohibited.  *This is intentional and
> will help you practice parsing strings and manipulating pointers.*

> :scream: You **MAY NOT** use dynamic memory allocation in this assignment
> (i.e. `malloc`, `realloc`, `calloc`, `mmap`, etc.).  There is one function
> (`argo_append_char()`) provided for you that does the dynamic allocation
> required while accumulating the characters of a string or numeric literal.
> This function is in the file `const.c`, which you are not to modify.

> :nerd: Reference for command line arguments: [https://beej.us/guide/bgc/html/#command-line-arguments](https://beej.us/guide/bgc/html/#command-line-arguments).

**NOTE:** The `make` command compiles the `argo` executable into the `bin` folder.
All commands from here on are assumed to be run from the `hw1` directory.

### **Required** Validate Arguments Function

In `global.h`, you will find the following function prototype (function
declaration) already declared for you. You **MUST** implement this function
as part of the assignment.

```c
int validargs(int argc, char **argv);
```

The file `validargs.c` contains the following specification of the required behavior
of this function:

```c
/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
```

> :scream: This function must be implemented as specified as it will be tested
> and graded independently. **It should always return -- the USAGE macro should
> never be called from validargs.**

The `validargs` function should return -1 if there is any form of failure.
This includes, but is not limited to:

- Invalid number of arguments (too few or too many).

- Invalid ordering of arguments.

- A missing parameter to an option that requires one [doesn't apply to the
  current assignment, since the parameter to `-p` is optional].

- Invalid parameter.  A numeric parameter specfied with `-p` is invalid if
  it does not conform to the format of a nonnegative integer as required by
  the JSON specification.

The `global_options` variable of type `int` is used to record the mode
of operation (i.e. encode/decode) of the program and associated parameters.
This is done as follows:

- If the `-h` flag is specified, the most significant bit (bit 31) is 1.

- If the `-v` flag is specified, the second-most significant bit (bit 30)
  is 1.

- If the `-c` flag is specified, the third-most significant bit (bit 29)
  is 1.

- If the `-p` flag is specified, the fourth-most significant bit (bit 28)
  is 1.

- The least significant byte (bits 7 - 0) records the number of spaces of
  indentation per level specified with `-p`, or the default value (4)
  if no value was specified with `-p`.  If `-p` was not specified at all,
  then this byte should be 0.

If `validargs` returns -1 indicating failure, your program must call
`USAGE(program_name, return_code)` and return `EXIT_FAILURE`.
**Once again, `validargs` must always return, and therefore it must not
call the `USAGE(program_name, return_code)` macro itself.
That should be done in `main`.**

If `validargs` sets the most-significant bit of `global_options` to 1
(i.e. the `-h` flag was passed), your program must call `USAGE(program_name, return_code)`
and return `EXIT_SUCCESS`.

> :nerd: The `USAGE(program_name, return_code)` macro is already defined for you
> in `argo.h`.

If validargs returns 0, then your program must read input data from `stdin`
and (depending on the options supplied) write output data to `stdout`.
Upon successful completion, your program should exit with exit status `EXIT_SUCCESS`;
otherwise, in case of an error it should exit with exit status `EXIT_FAILURE`.

Unless the program has been compiled for debugging (using `make debug`),
in a successful run that exits with `EXIT_SUCCESS` no other output may be produced
by the program.  In an unsuccessful run in which the program exits with `EXIT_FAILURE`
the program should output to `stderr` a one-line diagnostic message that indicates
the reason for the failure.  The program must not produce any other output than this
unless it has been compiled for debugging.

> :nerd: Remember `EXIT_SUCCESS` and `EXIT_FAILURE` are defined in `<stdlib.h>`.
> Also note, `EXIT_SUCCESS` is 0 and `EXIT_FAILURE` is 1.

### Example validargs Executions

The following are examples of the setting of `global_options` and the
other global variables for various command-line inputs.
Each input is a bash command that can be used to invoke the program.

- **Input:** `bin/argo -h`.  **Setting:** `global_options=0x80000000`
(`help` bit is set, other bits clear).

- **Input:** `bin/argo -v `.  **Setting:** `global_options=0x40000000`
(mode is "validate").

- **Input:** `bin/argo -c -p 2`.  **Setting:** `global_options=0x30000002`
(mode is "canonicalize", "pretty-print" has been specified with
indentation increment 2).

- **Input:** `bin/argo -p 2 -c`.  **Setting:** `global_options=0x0`.
This is an error case because the specified argument ordering is invalid
(`-p` is before `-c`). In this case `validargs` returns -1, leaving
`global_options` unset.

# Part 2: Overview of the JSON Specification

JSON ("JavaScript Object notation") is a standard format for data interchange
that is now commonly used in many areas of computing.
It was designed to be extremely simple to generate and parse and it in fact
achieves these goals: JSON syntax is about as simple as it gets for a
computer language that is actually used in the real world.
The syntax of JSON is defined by an
[ECMA standard](ECMA-404_2nd_edition_december_2017.pdf).
A summary that omits the scarier language from the standard document is given at
[www.json.org](https://www.json.org/json-en.html).
Most likely, you will only need to refer to this summary, but the full standard
document is here if you want to look at it.

In order to understand the JSON syntax specification, you need to be able to
read the "railroad diagrams" that are used to formally specify it.
These diagrams are actually a graphical version of a *context-free grammar*,
which is a standard tool used for formally specifying all kinds of computer
languages.  Actually, the white box inset on the right contains the full
grammar; the railroad diagrams only describe the portion of the syntax that
has any significant complexity.
Each of the railroad diagrams defines the syntax of a particular
"syntactic category", which is a set of strings having a similar format.
Examples of syntactic categories for JSON are "object", "array",
"value", "number", *etc*.
The paths along the "railroad tracks" in the diagram for one syntactic category
indicate the possibilities for forming a string in that category from strings
in other categories.
For example, the first diagram says that a string in the category "object"
always has an initial curly bracket `{`.  This may be followed immediately by
a closing curly bracket `}` (the top "track"), or between the brackets there
may be something more complicated (a list of "members" -- the lower "track").
By following the lower track, you find that there has to be "whitespace",
followed by a "string", followed by "whitespace", followed by a colon `:`,
followed by a "value".  After the "value", it is possible to have the
closing curly bracket `}` or to loop back around and have another instance of
the same pattern that was just seen (a "member").  The path to loop back around
requires that a comma `,` appear before the next member, so this tells you
that the members in between the `{` and `}` are separated by commas.
The other diagrams are read similarly, and even if you have never seen these
before, with a little study they should be self-explanatory so I'm not going
to belabor the explanation further.

Something that was not initially clear to me from just looking at the diagrams
was what the syntax of `true`, `false`, and `null` is.  These are shown with
double quotes in the inset box on the right, but in fact, the "token" `true`
simply consists of the four letters: `t`, `r`, `u`, `e` without any quotes.
This is spelled out better in the ECMA standard document.

The description of "character" in the inset box is also a bit mysterious
at first reading.  A "character" is something that is permitted to occur within
a string literal.  After staring at the description for awhile it becomes clear
that any Unicode code point except for (1) the "control characters"
whose code points range from U+0000 to U+001F, (2) the quote `"`,
and (3) the backslash '\' (they call it "reverse solidus"), may appear directly
representing themselves within a string literal.
In addition, "escape sequences" are permitted.  An escape sequence starts
with a backslash `\`, which may be followed by one of the characters
`"`, '\\', '/', 'b', 'f', 'n', 'r', 't', or 'u'.  After 'u' there are required
to appear exactly four hexadecimal digits, the letters of which may either
be in upper case or lower case.  The meaning of `\"`, `\/`, `\b`, `\f`
`\n`, `\r`, and `\t` is as in a C string.  The escape sequence `\/` represents
a single forward slash ("solidus") `/` (I do not know why this is in the
standard.)  The meaning of `\uHHHH`, where `HHHH` are four hex digits is
the 16-bit Unicode code point from the "basic multilingual plane" whose
value is given by interpreting `HHHH` as a four-digit hexadecimal number.

Although a Unicode code point outside the basic multilingual plane may
occur directly in a string literal, representing such by an escape requires
the use of a "surrogate pair" as indicated in the ECMA standard document.
Don't worry about this technicality.  For this assignment, your implementation
will not have to handle full Unicode and UTF-8-encoded input.
You may assume instead that the input to your program will come as a sequence of
8-bit bytes, each of which directly represents a Unicode code point in the
range U+0000 to U+00FF (the first 128 code points correspond to ASCII codes,
and the meaning of the next 128 code points is defined by the Unicode standard).
Note that this means that we are *not* using the usual UTF-8 encoding to
represent Unicode as a sequence of 8-bit bytes.
As you will see when you look at the definitions of the data structures you
are to use, internally your program will use the 32-bit `int`
(typedef'ed as `ARGO_CHAR`) to represent a character.
This is enough bits to represent any Unicode code point, so there will
be no problem in taking the input bytes that you read in and storing them
internally as Unicode code points.  Due to the limitation of the input encoding,
for us a string literal will not be able to directly contain any Unicode
code point greater than U+00FF. 
Nevertheless, you will still be able to use escape sequences within
a string literal to represent Unicode code points in the basic multilingual
plane (from U+0000 to U+FFFF), because the escape sequence allows you
to specify the code point directly as four hexadecimal digits.
Since we will also output JSON as a sequence of 8-bit bytes, it will be
necessary to render any Unicode code points greater than U+00FF occuring
in a string literal using escapes.

When reading a specification like this, it is helpful to have examples of
what is being defined.  For this purpose, I have provided (in the `rsrc`
directory) some sample JSON files.  These files all have the `.json`
extension.  Some of these files are examples of what your program is supposed
to do when given other files as input.  For example, the file `rsrc/numbers.json`
contains the following content.

```
{
    "0": 0,
    "2147483648": 2147483648,
    "-2147483649": 2147483649,
    "0.0": 0.0,
    "1": 1,
    "789": 789,
    "1.0": 1.0,
    "-1.0": -1.0,
    "1e3": 1e3,
    "1E3": 1E3,
    "1e-3": 1e-3,
    "1.234": 1.234,
    "-1.234": -1.234,
    "1.234e3": 1.234e3,
    "1.234e-3": 1.234e-3
}
```

when your program is run as follows

```
$ bin/argo -c -p 2 < rsrc/numbers.json
```

it should produce the output in `rsrc/numbers_-c_-p_2.json`; namely

```
{
  "0": 0,
  "2147483648": 2147483648,
  "-2147483649": 2147483649,
  "0.0": 0.0,
  "1": 1,
  "789": 789,
  "1.0": 0.1e1,
  "-1.0": -0.1e1,
  "1e3": 0.1e4,
  "1E3": 0.1e4,
  "1e-3": 0.1000000000000000e-2,
  "1.234": 0.1233999999999999e1,
  "-1.234": -0.1233999999999999e1,
  "1.234e3": 0.1233999999999999e4,
  "1.234e-3": 0.1234000000000000e-2
}
```

How this is supposed to happen is explained below.

# Part 3: Implementation

The header file `global.h` lists prototypes for functions you are
required to implement:

```c
ARGO_VALUE *argo_read_value(FILE *);
int argo_read_string(ARGO_STRING *s, FILE *);
int argo_read_number(ARGO_NUMBER *n, FILE *);

int argo_write_value(ARGO_VALUE *, FILE *);
int argo_write_string(ARGO_STRING *, FILE *);
int argo_write_number(ARGO_NUMBER *, FILE *);

int validargs(int argc, char **argv);
```

The `validargs()` function has already been discussed above.
The `argo_read_value()` function reads JSON input from the specified stream
and returns an `ARGO_VALUE` data structure (as described below).
The `argo_read_string()` function takes a pointer to an `ARGO_STRING`
structure (which will be a sub-structure of an `ARGO_VALUE` structure),
as well as a `FILE *` pointer, and it reads a JSON string literal
(starting and ending with a quote `"`) from the input stream and stores
the content of the string (without the quotes, after handling escapes)
in the specified `ARGO_STRING` object.
The `argo_read_number()` function works similarly, except it reads
a JSON numeric literal and uses it to initialize an `ARGO_NUMBER`
structure.

The `argo_write_value()` function takes an `ARGO_VALUE` data structure
and a `FILE *` pointer representing an output stream, and it writes
canonical JSON representing the specified value to the output stream.
The `argo_write_string()` function takes an `ARGO_STRING *` pointer
and a `FILE *` pointer and writes a JSON string literal to the output
stream (including quotes and escaping content that needs to be escaped).
The `argo_write_number()` function similarly takes an `ARGO_NUMBER *`
pointer and a `FILE *` pointer and it writes a JSON numeric literal
to the output stream.

> :scream: Even though your final application will only ever read JSON input
> from `stdin` and write JSON output to `stdout`, the interfaces of these
> functions are designed to accept arbitrary streams as parameters.
> **You must not ignore these parameters.**  Also, you must not assume that
> these streams are "seekable" and consequently you may not use the functions
> `fseek()` or `ftell()` in your code.

Besides the general discussion below, more detailed specifications for the
required behavior of these functions are given in the comments preceding
the (non-functional) stubs in `argo.c`.  Those specifications are mostly
not repeated here to avoid redundancy and possible inconsistencies between
this document and the specifications in `argo.c`.

Of course, you will also have to make modifications to the `main()` function,
so that after calling `validargs()` it makes the calls to
`argo_read_value()` and `argo_write_value()` to perform the functions required
of the complete application.

Since I want everybody to get the experience of designing and coding their
own implementation for this assignment, I have not spelled out any further
what other functions you will might to implement, but you will almost certainly
want to implement other functions.  Note that the function interfaces
that have been specified, together with the problems that have to be solved
by these functions, give you clues about an implementation structure that
you might wish to consider.  I will now discuss this briefly.

The `argo_read_value()` function is supposed to read bytes of data from a
stream and attempt to *parse* them as a JSON "value" (which could be
an object, array, string, number, or one of the basic tokens `true`,
`false` or `null`).  The result of this parsing process is a data structure
that represents the structure of the JSON in a form that is useful for
further processing.  The specification of the syntax has a recursive
structure (*e.g.* an object contains members, members contain elements, which
can themselves contain values, and so on.  A simple way to parse a string
according to a recursive specification like this is via a so-called
*recursive descent* parser.  Basically, the parser will have a function
for each of the syntactic categories that appear in the syntax specification
(`argo_read_value()` is one such function).  Each of these functions will
be called at a point where what is expected on the input stream is a string
belonging to the syntactic category handled by that function.
The function will read one or more characters from the input stream and, based
on what it sees, it will recursively call one or more of the other parser
functions.  For example, the function responsible for parsing an "object"
might check that the next character in the input is a curly brace `{`
and then call the function responsible for parsing a "member".
Each parsing function will return a data structure that represents what it
has parsed.  To build this data structure, each parsing function will
typically need make use of the data structures returned by the functions
that it called recursively.

In general, each function in a recursive descent parser will need to examine
a certain amount of the input in order to determine what to do.  This input
is called "look-ahead".  One of the features of the JSON syntax that makes
it so easy to parse is that at most one character of look-ahead is ever
required in order to decide what to do next.  For example, once we have
seen the `{` that starts an object, checking whether the next character is
a `}` or not is sufficient to tell whether we have to call functions
to parse members of the object, or whether the object is empty.
In implementing a parser like this, it generally simplifies the design
if you can "peek" at the look-ahead character without consuming it.
That way, when you call another function, it can assume that the input
stream is at the very start of what it is supposed to be trying to parse,
rather than having to keep track of what characters might already have
been been read by the caller.
You should use the `fgetc()` function from the C standard I/O library
to read each byte of data from the input stream.  This function consumes
the byte of data from the input stream, but the standard I/O library
also provides a function `ungetc()` that allows you to "push back" a single
character of input.  So you can achieve the effect of peeking one character
into the input stream by calling `fgetc()`, looking at the character returned,
and then using `ungetc()` to push it back into the stream if it is not
to be consumed immediately.  In some cases, as you descend through recursive
calls, the same character might be examined and pushed back repeatedly.

The recursive structure also dictates a natural form for the implementation
of the output function `argo_write_value()`: you can have one function
for each meaningful entity (*e.g.* "object", "member", "number") in the
JSON specification and these functions will call each other recursively
in order to traverse the data structure and emit characters to the output
stream.

# Part 4: Data Structures

The `argo.h` header file gives C definitions for the data structures you are
produce as the return values from `argo_read_value()` and as the arguments
to `argo_write_value()`.  These data structures are basically trees.
The `ARGO_VALUE` structure is the central definition, which spells out what
information is in a node of such a tree.  As the same `ARGO_VALUE` structure
is used to represent all the types of JSON values ("object", "array", "number",
*etc.*) it has a `type` field to indicate specifically what type of object
each individual instance represents.  The possible types are defined by the
`ARGO_VALUE_TYPE` enumeration.  Each node also has a `content` field, which
is where the actual content of the node is stored.  The `content` field
is defined using the C `union` type, which allows the same region of memory
to be used to store different types of things at different times.
Depending on what is in the `type` field, exactly one of the `object`, `array`,
`string`, `number`, or `basic` subfields of `content` will be valid.
Except for `ARGO_BASIC`, which just defines a set of possible values,
each of these has its own structure definition, which are given as
`ARGO_OBJECT`, `ARGO_ARRAY`, `ARGO_STRING`, and `ARGO_NUMBER`.

Besides the `type` and `content` fields, each `ARGO_VALUE` node contains
`next` and `prev` fields that point to other `ARGO_VALUES`.  These fields
will be used to link each node with other "sibling" nodes into a list.
For example, a JSON "object" has a list of "members".
The JSON object will be represented by an `ARGO_VALUE` node having
`ARGO_OBJECT_TYPE` in its `type` field.  The `content` field of this
object will therefore be used to hold an `ARGO_OBJECT` structure.
The `ARGO_OBJECT` structure has a single field: `member_list`, which
points to a "dummy" `ARGO_VALUE` structure used as the head of
a *circularly, doubly linked list* of members (more on this below).
Linked into this list will be `ARGO_VALUE` structures that represent
the members.  The `next` and `prev` fields of these are used to chain
the members into a list: the `next` field points from a member to
the next member and the `prev` field points from a member to the previous
member.  For `ARGO_VALUE` structures used to represent members of
an object, the `name` field will contain an `ARGO_STRING` structure
that represents the name of the member.

JSON arrays are represented similarly to JSON objects: the array as a
whole is represented by an `ARGO_VALUE` structure whose `type` field
contains `ARGO_ARRAY_TYPE`.  The `content` field will therefore be used
to hold an `ARGO_ARRAY` structure, which has an `element_list` field that
points to a "dummy" `ARGO_VALUE` structure at the head of a list of elements,
similarly to what was just described for for object members.
However, array elements don't have names, so the `name` field of each
array element will just be `NULL`.

JSON strings are represented by the `ARGO_STRING` structure, which
has fields `capacity`, `length`, and `content`.  These are used to
represent a dynamically growable string, similarly to the way
`ArrayList` is implemented in Java.
At any given time, the `content` field will either be `NULL`
(if the string is empty) or it will point to an array of `ARGO_CHAR`
elements, each of which represents a single Unicode code point.
The `capacity` field tells the total number of "slots" in this
array, whereas the `length` field tells how many of these are
actually used (*i.e.* it gives the current length of the string).
For this assignment, you don't have to actually be concerned with
the dynamic allocation -- that is performed by the function
`argo_append_char()` which has been implemented for you in `const.c`.
All you have to worry about is making sure that the fields
of and `ARGO_STRING` structure that you want to use have been
initialized to zero and then you can just call `argo_append_char()`
to build the string content.
A simple `for` loop using the `length` field as the upper limit
can then be used to traverse the `content` array of an `ARGO_STRING`
once it has been initialized.

The `ARGO_NUMBER` structure is used to represent a number.
One of its fields is a `string_value` field, which is an `ARGO_STRING`
used to hold the digits and other characters that make up the
textual representation of the number.  During parsing, characters
are accumulated in this field using `argo_append_char()` in the
same way that characters are accumulated for a string value.
The remaining fields (`int_value`, `float_value`) are used to store
an internal representation (either integer or floating-point)
of the value of the number, as well as flags (`valid_string`,
`valid_int`, `valid_float`) that tell which of the other fields
contain valid information.  Note that a JSON number that contains
a fractional part or an exponent part will generally not be representable
in integer format, so the `valid_int` field should be zero and there
will be no useful information in the `int_value` field.  Also, if an
`ARGO_NUMBER` is created internally, without parsing it from an input
stream, then a printable representation has not yet been computed, so the
`valid_string` field will be zero and the `string_value` field
will represent an empty string.

To summarize, your `argo_read_value()` function will read bytes of
data from the specified input stream using `fgetc()`.
As it reads and parses the input, it will build up a tree of
`ARGO_VALUE` nodes to represent the structure of the JSON input.
The nodes of the resulting tree must satisfy the following requirements:

- A node with `ARGO_OBJECT_TYPE` in its `type` field represents
  a JSON "object".  The `content` field then contains an `ARGO_OBJECT`
  structure whose `member_list` field points to an `ARGO_VALUE`
  node that is the head of a circular, doubly linked list of members.
  Each member has a pointer to its associated name (an `ARGO_STRING`)
  stored in the `name` field.

- A node with `ARGO_ARRAY_TYPE` in its `type` field represents
  a JSON "array".  The `content` field then contains an `ARGO_ARRAY`
  structure whose `element_list` field points to an `ARGO_VALUE`
  node that is the head of a circular, doubly linked list of elements.

- A node with `ARGO_STRING_TYPE` in its `type` field represents
  a JSON "string" (without the enclosing quotes that appear in JSON
  source).  The `content` field then contains an `ARGO_STRING`
  that represents the string.  The `length` field of the `ARGO_STRING`
  gives the length of the string and the `content` field points to
  an array of `ARGO_CHAR` values that are the content of the string.

- A node with `ARGO_NUMBER_TYPE` in its `type` field represents
  a JSON "number".  The `content` field then contains an `ARGO_NUMBER`
  object that represents the number in various ways.

  * If the `valid_string` field is nonzero, then the `string_value`
    field will contain an `ARGO_STRING` that holds the characters that
    make up a printable/parseable representation of the number.

  * If the `valid_int` field is nonzero, then the `int_value`
    field will contain the value of the number as a C `long`.

  * If the `valid_float` field is nonzero, then the `float_value`
    field will contain the value of the number as a C `double`.

  If there is more than one representation of the number present,
  then they are required to agree with each other (*i.e* represent
  the same value).

- A node with `ARGO_BASIC_TYPE` in its `type` field will have
  a `content` field having a value of type `ARGO_BASIC` in its `basic`
  field.  This value will be one of `ARGO_TRUE`, `ARGO_FALSE`,
  or `ARGO_NULL`.

The `argo_read_string()` function will parse a JSON string literal
and store the string content into an `ARGO_STRING` object.
Characters in the input that are not control character and are not
one of the characters that must be escaped are simply appended directly
to the string content.  However when a backslash `\` is encounted,
it is necessary to interpret it as the start of an *escape sequence*
that represents the character to be appended.  These escape sequences
should be familiar, since they are essentially the same as those
used in Java as well as in C.

The `argo_read_number()` function will parse a JSON numeric literal
and store into an `ARGO_NUMBER` object not only the sequence of
characters that constitute the literal, but also the value of the
number, either in integer format, in floating point format, or both.
In order to do this, you have to actually process the various digits
one at a time and calculate (using integer and/or floating point
arithemetic) the value that is represented.  You have to carry out
this conversion yourself; you are not allowed to use any library
functions to do it.

## Circular, Doubly Linked Lists

As already stated, object members and array elements are to be stored as
circular, doubly linked lists of `ARGO_VALUE` structures, using a "dummy"
structure as a sentinel.  Even though the sentinel has the same type
(*i.e.* `ARGO_VALUE`) as the elements of the list, it does not itself represent
an element of the list.  The only fields used in the sentinel are the
`next` field, which points to the first actual element of the list,
and the `prev` field, which points to the last actual element of the list.
The list is "circular" because starting at the sentinel and following
`next` pointers will eventually lead back to the sentinel again.
Similarly, starting at the sentinel and following `prev` pointers will
eventually lead back to the sentinel.  An empty list is represented
by a sentinel whose `next` and `prev` pointers point back to the sentinel
itself.  You can read more about this type of data structure by searching,
e.g. Wikipedia for "circularly doubly linked list".  The advantage of
using the sentinel is that all insertions and deletions are performed
in exactly the same way, without any edge cases for the first or last
element of the list.

## Dynamic Storage

There are two types of dynamic storage used by this program.
One of these is for the content of an `ARGO_STRING`.  As already indicated
above, this is handled for you by the function `argo_append_char()` and you
do not have to worry about how it happens.
The other dynamic storage is for `ARGO_VALUE` structures.
You need a source of such structures while you are building the tree
that represents JSON.
As you are prohibited from declaring your own arrays in this
assignment, you will have to use one that we have already declared for you.
In `global.h` an array `argo_value_storage` has been defined for you,
together with an associated counter `argo_next_value`.  You **must** use
this array as the source of `ARGO_VALUE` structures for building your
JSON trees.  Use the `argo_next_value` counter to keep track of the
index of the first unused element of this array.
When you need an `ARGO_VALUE` structure, get a pointer to the first unused
element of the `argo_value_storage` array and increment `argo_next_value`.
Be sure to pay attention to the total number `NUM_ARGO_VALUES` of
elements of this array -- if you run off the end you will corrupt other
memory and your program will have unpredictable behavior.

# Part 5: Canonical Output

Your `argo_write_value()` function is supposed to traverse a data structure
such as that returned by `argo_read_value()` and it is supposed to output
JSON to the output stream.  First of all, the JSON that you output has to
conform to the JSON standard, so that it can be parsed again to produce
exactly the same internal data structure.  Beyond that, the JSON is supposed
to be "canonical", which means that it has been output in a standard way
that does not leave any possibility for variation.
Your canonical JSON output must always satisfy the following conditions:

- An `ARGO_NUMBER` whose `valid_int` field is set is to be printed out as
  an integer, without any fraction or exponent.

- An `ARGO_NUMBER` whose `valid_float` field is set is to be printed out
  with an integer and fractional part, as in the JSON specification.
  The fractional part should be normalized to lie in the interval `[0.1, 1.0)`,
  so that there is always just a single `0` digit before the decimal point
  and the first digit after the decimal point is always nonzero.
  An exponent of 0 is to be omitted completely and for positive exponents
  the `+` sign is to be omitted.  Exponents always start with lower-case `e`,
  rather than upper-case `E`.

- An `ARGO_STRING` is printed so that the following conditions are satisfied:

  * Characters (other than backslash `\` and quote `"`) having Unicode control
    points greater than U+001F and less than U+00FF are to appear directly
    in the string literal as themselves.  This includes forward slash `/`.

  * Characters with Unicode code points greater than U+00FF
    are to appear as escapes using `\u` and the appropriate hex digits,
    which must be in lower case.
	
  * Control characters that have special escapes (`\n`, `\t`, *etc.*) must
	be printed using those special escapes, not using the generic escape `\u`
    with hex digits.
	

If the pretty-print option has not been specified, then your canonical JSON
output must satisfy the following condition:

- There is no white space in the output, except for white space that occurs
  within a string literal.

If the pretty print option has been specified, then your canonical JSON output
will include white space according to the following rules:

- A single newline is output after every `ARGO_VALUE` that is output at the
  top-level (*i.e.* not as part of an object or array).

- A single newline is output after every '{', '[', and ',' (except those in string
  literals).

- A single newline is output immediately after the last member of an object,
  and immediately after the last element of an array.

- Each newline is followed by a number of spaces that depends on the indentation
  level of the value currently being printed.  The indentation level is maintained
  as follows:

  * The indentation level is 0 for a top-level JSON value.

  * The indentation level is increased by one just after a `{` or `[` has
    been printed to start the list of members of an object or elements of
    an array.
    The indentation level decreases by one just after the last member or
	element has been printed, so that the closing `}` or `]` is at the
	previous indentation level

  * A single space is printed following each colon `:` that separates
	the name of an object member from its value.

  The number of spaces following a newline is equal to the current indentation
  level times the `INDENT` argument given with `-p`, or the default of `4`
  if `-p` was specified without an `INDENT` argument.

Note that canonicalization must be an "idempotent" operation, in the sense that
if canonical output previously produced is re-parsed and then re-output using
the same pretty-printing settings, then the new output should be identical
to the previous output.

# Part 6: Strategy of Attack

To make things a little easier for you in getting started on this assignment,
I have distributed with the basecode a library containing binary object
versions of my own implementations of `argo_read_value()` and `argo_write_value()`.
The `Makefile` has been constructed so that it will link your program against
the library I provided.  As a result, if you comment out one or both of
these function in `argo.c`, my versions will be linked instead and you can
use them to work on the other parts of the assignment.  Note that this library
will **not** be present during grading, so do not leave these functions
commented out or your code will not compile.

Note that the functions whose interfaces have been specified will likely
be unit-tested.  This means that their behavior should be completely determined
by their specified interface, which includes their parameters, return values,
and global variables defined in `global.h` (which you may **not** modify).
There should be no implicit assumption that any other functions have been or
will be called or that any particular variables have been set to any particular
values, except for the global variables defined in `global.h`.
So, for example, you may (and should) assume that when `argo_write_object()`
is called, the `global_options` variable has been set according to the desired
program options, but you may **not** assume that before `argo_write_object()`
has been called that some other function was called previously.

My best guess as to the right attack strategy for this assignment is as follows:
First, work on the command-line argument processing (`validargs()`) and
make the changes to `main()` necessary to get the program to honor the command-line
arguments and perform the overall function that the application is supposed
to perform.
Next, start working on implementing `argo_write_value()`, using my version of
`argo_read_value()` as a source of data structures that you can use to increase
your understanding of pointers and the specific data structures that we are using
to represent JSON and, ultimately, as an aid to developing and testing your
implementation.
Finally, now that you have a clear understanding of the data structures you
are trying to produce work on implementing `argo_read_value()`, to parse
a stream of input bytes and produce such a data structure.  I expect this part
of the assignment to be the most difficult.

Note that the code that I wrote for `argo_read_value()` and `argo_write_value()`
is only about 800 lines in length.  If you find your own code growing
much larger than that, you need to step back and think smarter about trying
to simplify your code.

# Part 7: Running the Program

The `argo` program always reads from `stdin` and possibly writes to `stdout`.
If you want the program to take input from a file or produce output to
a file, you may run the program using **input and output redirection**,
which is implemented by the shell.
A simple example of a command that uses such redirection is the following:

```
$ bin/argo -c < rsrc/numbers.json > numbers.out
```

This will cause the input to the program to be redirected from the text file
`rsrc/numbers.json` and the output from the program to be redirected to the
file `numbers.out`.
The redirection is accomplished by the shell, which interprets the `<` symbol
to mean "input redirection from a file" and the `>` symbol to mean
"output redirection to a file".  It is important to understand that redirection
is handled by the shell and that the `bin/argo` program never sees any
of the redirection arguments; in the above example it sees only `bin/argo -c`
and it just reads from `stdin` and writes to `stdout`.

Alternatively, the output from a command can be **piped**
to another program, without the use of a disk file.
This could be done, for example, by the following command:

```
$ bin/argo -c -p 2 < rsrc/package-lock.json | less
```

This sends the (rather lengthy) output to a program called `less`,
which display the first screenful of the output and then gives you the ability
to scan forward and backward to see different parts of it.
Type `h` at the `less` prompt to get help information on what you can do
with it.  Type `q` at the prompt to exit `less`.

Programs that read from standard input and write to standard output are
often used as components in more complex "pipelines" that perform multiple
transformations on data.

For example, one way to test your implementation is by using one instance
of it to produce some output and testing to see if that output can be read by
another instance; *e.g.:

```
$ cat rsrc/package-lock.json | bin/argo -c | bin/argo -c -p 2 > p.out
```

Here `cat` (short for "concatenate") is a command that reads the files
specified as arguments, concatenates their contents, and prints the
concatenated content to `stdout`.  In the above command, this output
is redirected through a pipe to become the input to `bin/argo -c`.
The output of `bin/argo -c` (which contains no whitespace) is then
sent to `bin/argo -c -p 2` for pretty printing.  Finally, the pretty-printed
output is written to file `p.out`.  Actually, the original input
file `rsrc/package-lock.json` is already canonical as defined here,
so in the end the file `p.out` should have exactly the same content
as `rsrc/package-lock.json`.  One way to check this is to use the
`diff` comand (use `man diff` to read the manual page) to compare the
two files:

```
$ diff rsrc/package-lock.json p.out
$
```

If `diff` exits silently, the files are identical.
Another command that would be useful on output with no whitespace
is the `cmp` command, which performes a byte-by-byte comparison of two files
(even files that contain raw binary data):

```
$ cmp rsrc/package-lock.json p.out
```

If the files have identical content, `cmp` exits silently.
If one file is shorter than the other, but the content is otherwise identical,
`cmp` will report that it has reached `EOF` on the shorter file.
Finally, if the files disagree at some point, `cmp` will report the
offset of the first byte at which the files disagree.
If the `-l` flag is given, `cmp` will report all disagreements between the
two files.

## Unit Testing

Unit testing is a part of the development process in which small testable
sections of a program (units) are tested individually to ensure that they are
all functioning properly. This is a very common practice in industry and is
often a requested skill by companies hiring graduates.

> :nerd: Some developers consider testing to be so important that they use a
> work flow called **test driven development**. In TDD, requirements are turned into
> failing unit tests. The goal is then to write code to make these tests pass.

This semester, we will be using a C unit testing framework called
[Criterion](https://github.com/Snaipe/Criterion), which will give you some
exposure to unit testing. We have provided a basic set of test cases for this
assignment.

The provided tests are in the `tests/basecode_tests.c` file. These tests do the
following:

- `validargs_help_test` ensures that `validargs` sets the help bit
correctly when the `-h` flag is passed in.

- `validargs_validate_test` ensures that `validargs` sets the validate-mode bit
correctly when the `-v` flag is passed.

- `validargs_canonicalize_test` ensures that `validargs` sets the canonicalize-mode bit
correctly when the `-c` flag is passed in.

- `validargs_bits_test` ensures that `validargs` sets the decode-mode bit
correctly when the `-d` flag is passed in and that the value passed with `-b`
is correctly stored in the least-signficant byte of `global_options`.

- `validargs_error_test` ensures that `validargs` returns an error when the `-p`
flag is supplied with the `-v` flag.

- `help_system_test` uses the `system` syscall to execute your program through
Bash and checks to see that your program returns with `EXIT_SUCCESS`.

- `argo_basic_test` performs a basic test of the canonicalization mode of the program.

### Compiling and Running Tests

When you compile your program with `make`, an `argo_tests` executable will be
created in your `bin` directory alongside the `argo` executable. Running this
executable from the `hw1` directory with the command `bin/argo_tests` will run
the unit tests described above and print the test outputs to `stdout`. To obtain
more information about each test run, you can use the verbose print option:
`bin/argo_tests --verbose=0`.

The tests we have provided are very minimal and are meant as a starting point
for you to learn about Criterion, not to fully test your homework. You may write
your own additional tests in `tests/basecode_tests.c`, or in additional source
files in the `tests` directory.  However, this is not required for this assignment.
Criterion documentation for writing your own tests can be
found [here](http://criterion.readthedocs.io/en/master/).

Note that grades are assigned based on the number of our own test cases
(not given to you in advance) that your program passes.
So you should work on the assignments in such a way that whatever you do submit
will function.  Code that is completely broken will not score any points,
regardless of how voluminous it might be or how long you might have spent on it.

## Sample Input Files

In the `rsrc` directory I have placed a few JSON input files for you to try
your code on.

- `numbers.json`:  A JSON file containing a single object with various
  numbers as its members.  This will exercise most (but probably not all)
  of the interesting cases that come up in parsing and outputting numbers.
  
- `strings.json`: A JSON file containing a single array with various
  strings as its elements.  These are intended to exercise most (but again,
  probably not all) of the cases involving escape sequences in strings.
  
- `package-lock.json`:  This is a larger JSON file that I had lying around
  which seemed to be a reasonable overall test.

# Hand-in instructions

**TEST YOUR PROGRAM VIGOROUSLY BEFORE SUBMISSION!**

Make sure that you have implemented all the required functions specifed in `const.h`.

Make sure that you have adhered to the restrictions (no array brackets, no prohibited
header files, no modifications to files that say "DO NOT MODIFY" at the beginning,
no functions other than `main()` in `main.c`) set out in this assignment document.

Make sure your directory tree looks basically like it did when you started
(there could possibly be additional files that you added, but the original organization
should be maintained) and that your homework compiles (you should be sure to try compiling
with both `make clean all` and `make clean debug` because there are certain errors that can
occur one way but not the other).

This homework's tag is: `hw1`

`$ git submit hw1`

> :nerd: When writing your program try to comment as much as possible. Try to
> stay consistent with your formatting. It is much easier for your TA and the
> professor to help you if we can figure out what your code does quickly!

