# Homework 2 Debugging and Fixing - CSE 320 - Spring 2022
#### Professor Eugene Stark

### **Due Date: Friday 3/4/2022 @ 11:59pm**

# Introduction

In this assignment you are tasked with updating an old piece of
software, making sure it compiles, and that it works properly
in your VM environment.

Maintaining old code is a chore and an often hated part of software
engineering. It is definitely one of the aspects which are seldom
discussed or thought about by aspiring computer science students.
However, it is prevalent throughout industry and a worthwhile skill to
learn.  Of course, this homework will not give you a remotely
realistic experience in maintaining legacy code or code left behind by
previous engineers but it still provides a small taste of what the
experience may be like.  You are to take on the role of an engineer
whose supervisor has asked you to correct all the errors in the
program, plus add additional functionality.

By completing this homework you should become more familiar
with the C programming language and develop an understanding of:

- How to use tools such as `gdb` and `valgrind` for debugging C code.
- Modifying existing C code.
- C memory management and pointers.
- Working with files and the C standard I/O library.

## The Existing Program

Your goal will be to debug and extend an old program called `par`,
which was written by Adam M. Costello and posted to Usenet in 1993.
I have rearranged the original source code and re-written the `Makefile`
to conform to the format we are using for the assignments in this course.
Besides a bug that was present in the original version, I have introduced
a few additional bugs here and there to make things more interesting
and educational for you :wink:.
Although you will need to correct these bugs in order to make the program
function, they do not otherwise change the program behavior from what
the author intended.

The `par` program is a simple paragraph reformatter.  It is basically
designed to read text from the standard input, parse the text into
paragraphs, which are delimited by empty lines, chop each paragraph up
into a sequence of words (forgetting about the original line breaks),
choose new line breaks to optimize some criteria that are designed to
produce a pleasing result, and the finally output the paragraph with
the new line breaks.  There are several parameters that can be set
which affect the result:  the width of the output text, the length of
a "prefix" and a "suffix" to be prepended and appended to each output line,
a parameter "hang", which affects the default value of "prefix", and
a boolean parameter "last", which affects the way the last line of a
paragraph is treated.

What you have to do is to first get the program to compile (for the most part,
I did not modify the original code, which requires some changes for it
to compile cleanly with the compiler and settings we are using).
Then, you need to test the program and find and fix the bugs that prevent it
from functioning properly.  Some of the bugs existed in the original version and
some I introduced for the purposes of this assignment.
Finally, you will make some modifications to the program.

As you work on the program, limit the changes you make to the minimum necessary
to achieve the specified objectives.  Don't rewrite the program;
assume that it is essentially correct and just fix a few compilation errors and
bugs as described below.  You will likely find it helpful to use `git` for this (I did).
Make exploratory changes first on a side branch (*i.e.* not the master branch),
then when you think you have understood the proper changes that need to be made,
go back and apply those changes to the master branch.  Using `git` will help you
to back up if you make changes that mess something up.

### Getting Started - Obtain the Base Code

Fetch base code for `hw2` as you did for the previous assignments.
You can find it at this link:
[https://gitlab02.cs.stonybrook.edu/cse320/hw2](https://gitlab02.cs.stonybrook.edu/cse320/hw2).

Once again, to avoid a merge conflict with respect to the file `.gitlab-ci.yml`,
use the following command to merge the commits:

<pre>
  git merge -m "Merging HW2_CODE" HW2_CODE/master --strategy-option=theirs
</pre>

  > :nerd: I hope that by now you would have read some `git` documentation to find
  > out what the `--strategy-option=theirs` does, but in case you didn't :angry:
  > I will say that merging in `git` applies a "strategy" (the default strategy
  > is called "recursive", I believe) and `--strategy-option` allows an option
  > to be passed to the strategy to modify its behavior.  In this case, `theirs`
  > means that whenever a conflict is found, the version of the file from
  > the branch being merged (in this case `HW2_CODE/master`) is to be used in place
  > of the version from the currently checked-out branch.  An alternative to
  > `theirs` is `ours`, which makes the opposite choice.  If you don't specify
  > one of these options, `git` will leave conflict indications in the file itself
  > and it will be necessary for you to edit the file and choose the code you want
  > to use for each of the indicated conflicts.

Here is the structure of the base code:

<pre>
.
├── .gitlab-ci.yml
└── hw2
    ├── doc
    │   ├── par.1
    │   ├── par.doc
    │   └── protoMakefile
    ├── hw2.sublime-project
    ├── include
    │   ├── buffer.h
    │   ├── debug.h
    │   ├── errmsg.h
    │   └── reformat.h
    ├── Makefile
    ├── rsrc
    │   ├── banner.txt
    │   ├── gettysburg.txt
    │   └── loremipsum.txt
    ├── src
    │   ├── buffer.c
    │   ├── errmsg.c
    │   ├── main.c
    │   ├── par.c
    │   └── reformat.c
    ├── test_output
    │   └── .git-keep
    └── tests
        ├── basecode_tests.c
        ├── rsrc
        │   ├── banner.txt
        │   ├── basic.in -> gettysburg.txt
        │   ├── basic.out
        │   ├── blank_lines.txt
        │   ├── EOF.in
        │   ├── EOF.out
        │   ├── gettysburg.txt
        │   ├── loremipsum.txt
        │   ├── prefix_suffix.in -> banner.txt
        │   ├── prefix_suffix.out
        │   ├── valgrind_leak.in -> gettysburg.txt
        │   ├── valgrind_leak.out
        │   ├── valgrind_uninitialized.err
        │   ├── valgrind_uninitialized.in -> loremipsum.txt
        │   └── valgrind_uninitialized.out
        ├── test_common.c
        └── test_common.h
</pre>

The `src` directory contains C source code files `buffer.c`. `par.c`, `reformat.c`,
and `errmsg.c`, which were part of the original code.  In addition, I have added
a new file `main.c`, with a single `main()` function that simply calls
`original_main()` in `par.c`.  This is to satisfy our requirement (for Criterion)
that `main()` is the only function in `main.c`.

The `include` directory contains C header files `buffer.h`, `reformat.h`, and
`errmsg.h`, which were part of the original source code.  I have also added our
`debug.h` header file which may be of use to you.

The `doc` directory contains documentation files that were part of the original
distribution of `par`.  The file `par.1` is in the format traditionally used
for Unix manual pages.  This file `par.` is intended to be processed with the
the formatting program `nroff` with argument `-man`; for example:
`nroff -man doc/par.1 | less` could be used to format and view its contents.

The `tests` directory contains C source code (in file `basecode_tests.c`) for some Criterion
tests that can help guide you toward bugs in the program.  These are not guaranteed
to be complete or exhaustive.  The `test_common.c` and `test_common.h` contain auxiliary code
used by the tests.  The subdirectory `tests/rsrc` contains input files and reference output files
that are used by the tests.
The `par` program was not designed to be particularly conducive to unit testing,
so all the tests we will make (including the tests used in grading) will be so-called
"black box" tests, which test the input-output behavior of the program running as a
separate process from the test driver.
The `test_common.c` file contains helper functions for launching an instance of `par`
as a separate process, redirecting `stdin` from an input file, collecting the
output produced on `stdout` and `stderr`, checking the exit status of the program,
and comparing the output against reference output.

The `test_output` directory is a "dummy" directory which is used to hold the output
produced when you run the Criterion tests.  Look there if you want to understand,
for example, why the tests reported that the output produced by your program was
not as expected.

Before you begin work on this assignment, you should read the rest of this
document.  In addition, we additionally advise you to read the
[Debugging Document](DebuggingRef.md).  One of the main goals of this assignment
is to get you to learn how to use the `gdb` debugger, so you should right away
be looking into how to use this while working on the tasks in the following sections.

# Part 1: Debugging and Fixing

You are to complete the following steps:

1. Clean up the code; fixing any compilation issues, so that it compiles
   without error using the compiler options that have been set for you in
   the `Makefile`.
   Use `git` to keep track of the changes you make and the reasons for them, so that you can
   later review what you have done and also so that you can revert any changes you made that
   don't turn out to be a good idea in the end.

2. Fix bugs.

    Run the program, exercising the various options, and look for cases in which the program
    crashes or otherwise misbehaves in an obvious way.  We are only interested in obvious
    misbehavior here; don't agonize over program behavior that might just have been the choice
    of the original author.  You should use the provided Criterion tests to help point the way,
	though they are not guaranteed to be exhaustive.

3. Use `valgrind` to identify any memory leaks or other memory access errors.
   Fix any errors you find.

    Run `valgrind` using a command of the following form:

    <pre>
      $ valgrind --leak-check=full --show-leak-kinds=all --undef-value-errors=yes [PAR PROGRAM AND ARGS]
    </pre>

    Note that the bugs that are present will all manifest themselves in some way
    either as incorrect output, program crashes or as memory errors that can be
	detected by `valgrind`.  It is not necessary to go hunting for obscure issues
	with the program output.
    Also, do not make gratuitous changes to the program output, as this will
    interfere with our ability to test your code.

   > :scream:  The author of this program was pretty fastidious about freeing memory before
   > exiting the program.  Once you have fixed the bugs, the program should exit without
   > any type of memory leak reported by `valgrind`, including memory that is "still reachable"
   > at the time of exit.  "Still reachable" memory corresponds to memory that is in use
   > when the program exits and can still be reached by following pointers from variables
   > in the program.  Although some people consider it to be untidy for a program
   > to exit with "still reachable" memory, it doesn't cause any particular problem.
   > For the present program, however, there should not be any "still reachable" memory.

   > :scream: You are **NOT** allowed to share or post on PIAZZA
   > solutions to the bugs in this program, as this defeats the point of
   > the assignment. You may provide small hints in the right direction,
   > but nothing more.

# Part 2: Changes to the Program

## Rewrite/Extend Options Processing

The basecode version of `par` performs its own *ad hoc* processing of command-line options.
This is likely due to the fact that there did not exist a commonly accepted library
package for performing this function at the time the program was written.
However, as options processing is a common function that is performed by most programs,
and it is desirable for programs on the same system to be consistent in how they interpret
their arguments, there have been more elaborate standardized libraries that have been written
for this purpose.  In particular, the POSIX standard specifies a `getopt()` function,
which you can read about by typing `man 3 getopt`.  A significant advantage to using a
standard library function like `getopt()` for processing command-line arguments,
rather than implementing *ad hoc* code to do it, is that all programs that use
the standard function will perform argument processing in the same way
rather than having each program implement its own quirks that the user has to remember.

For this part of the assignment, you are to replace the original argument-processing
code in `main()` by code that uses the GNU `getopt` library package.
In addition to the POSIX standard `getopt()` function, the GNU `getopt` package
provides a function `getopt_long()` that understands "long forms" of option
arguments in addition to the traditional single-letter options.
In your revised program, `main()` should use `getopt_long()` to traverse the
command-line arguments, and it should support the following option syntax
(in place of what was originally used by the program):

  - `--version` (long form only):
    Print the version number of the program.

  - `-w WIDTH` (short form) or `--width WIDTH` (long form):
    Set the output paragraph width to `WIDTH`.

  - `-p PREFIX` (short form) or `--prefix PREFIX` (long form):
    Set the value of the "prefix" parameter to `PREFIX`.

  - `-s SUFFIX` (short form) or `--suffix SUFFIX` (long form):
    Set the value of the "suffix" parameter to `SUFFIX`.

  - `-h HANG` (short form) or `--hang HANG` (long form):
    Set the value of the "hang" parameter to `HANG`.

  - `-l LAST` (short form) or either `--last` or
    `--no-last` (long form):
    Set the value of the boolean "last" parameter.
   For the short form, the values allowed for `LAST` should be either
   `0` or `1`.

  - `-m MIN` (short form) or either `--min` or `--no-min` (long form).
   Set the value of the boolean "min" parameter.
   For the short form, the values allowed for `MIN` should be either
   `0` or `1`.

You will probably need to read the Linux "man page" on the `getopt` package.
This can be accessed via the command `man 3 getopt`.  If you need further information,
search for "GNU getopt documentation" on the Web.

> :scream: You MUST use the `getopt_long()` function to process the command line
> arguments passed to the program.  Your program should be able to handle cases where
> the (non-positional) flags are passed IN ANY order.  Make sure that you test the
> program with prefixes of the long option names, as well as the full names.

## Revise the Error Message Scheme

The original program uses a very *ad hoc* scheme for error-message reporting:
if an error occurs, a string describing the error is stored into a global
character array `errmsg` with a hard-coded maximum size.  (This hard-coded
size has an occurrence in the `fprintf()` format string in `par.c`,
which creates undesirable implicit coupling between `par.c` and `errmsg.c`.)
At various points in the program, the existence of an error condition is checked
by looking to see if the first character of the error message string is a null
character `'\0'`.  Before the program terminates, if an error message exists,
then it is printed and the program exits with an error status, otherwise it exits
with a success indication.

Your job is to revise the error message scheme to make it somewhat more general
and to eliminate the hard-coded limitation on the length of an error message.
In particular, you should replace the interface defined in `errmsg.h` by the
following function prototypes (exactly as shown):

```c
/**
 * @brief  Set an error indication, with a specified error message.
 * @param msg Pointer to the error message.  The string passed by the caller
 * will be copied.
 */
void set_error(char *msg);

/**
 * @brief  Test whether there is currently an error indication.
 * @return 1 if an error indication currently exists, 0 otherwise.
 */
int is_error();

/**
 * @brief  Issue any existing error message to the specified output stream.
 * @param file  Stream to which the error message is to be issued.  
 * @return 0 if either there was no existing error message, or else there
 * was an existing error message and it was successfully output.
 * Return non-zero if the attempt to output an existing error message
 * failed.
 */
int report_error(FILE *file);

/**
 * Clear any existing error indication and free storage occupied by
 * any existing error message.
 */
void clear_error();
```

The global array `errmsg` should be removed from `errmsg.h` and replaced
by a pointer variable declared as `static char *` in `errmsg.c`.
The functions whose prototypes are given above should be implemented so
that there is no fixed maximum imposed on the length of an error message.
This means that error messages should be dynamically allocated on the
heap (for example, using `strdup()`).  The implementation should take care
not to leak any memory used for error messages; for example if a new error
message is set when one already exists.  Before exiting, the program should
call `clear_error()` to cause any existing error message to be freed.

# Part 3: Testing the Program

For this assignment, you have been provided with a basic set of
Criterion tests to help you debug the program.

In the `tests/basecode_tests.c` file, there are five test examples.
You can run these with the following command:

<pre>
    $ bin/par_tests
</pre>

To obtain more information about each test run, you can supply the
additional option `--verbose=1`.
You can also specify the option `-j1` to cause the tests to be run sequentially,
rather than in parallel using multiple processes, as is the default.
The `-j1` flag is necessary if the tests could interfere with each other in
some way if they are run in parallel (such as writing the same output file).
You will probably find it useful to know this; however the basecode tests have
been written so that they each use output files named after the test and
(hopefully) will not interfere with each other.

The tests have been constructed so that they will point you at most of the
problems with the program.
Each test has one or more assertions to make sure that the code functions
properly.  If there was a problem before an assertion, such as a "segfault",
the test will print the error to the screen and continue to run the
rest of the tests.
The basecode test cases check the program operation by reading input from
a pre-defined input file, redirecting `stdout` and `stderr` to output files,
and comparing the output produced against pre-defined reference files.
Some of the tests use `valgrind` to verify that no memory errors are found.
If errors are found, then you can look at the log file that is left behind
(in the `test_output` directory) by the test code.
Alternatively, you can better control the information that `valgrind` provides
if you run it manually.

The tests included in the base code are not true "unit tests", because they all
run the program as a black box using `system()`.
You should be able to follow the pattern to construct some additional tests of
your own, and you might find this helpful while working on the program.
You are encouraged to try to write some of these tests so that you learn how
to do it.  Note that in the next homework assignment unit tests will likely
be very helpful to you and you will be required to write some of your own.
Criterion documentation for writing your own tests can be found
[here](http://criterion.readthedocs.io/en/master/).

  > :scream: Be sure that you test non-default program options to make sure that
  > the program does not crash or otherwise misbehave when they are used.

# Hand-in Instructions

Ensure that all files you expect to be on your remote repository are committed
and pushed prior to submission.

This homework's tag is: `hw2`

<pre>
$ git submit hw2
</pre>
