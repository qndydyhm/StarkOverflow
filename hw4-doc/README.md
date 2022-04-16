# Homework 4 Scripting Language - CSE 320 - Spring 2022
#### Professor Eugene Stark

### **Due Date: Friday 4/15/2022 @ 11:59pm**

## Introduction

The goal of this assignment is to become familiar with low-level Unix/POSIX system
calls related to processes, signal handling, files, and I/O redirection.
You will implement an interpreter, called `mush`, for a simple scripting language
that is capable of managing multiple concurrently executing "jobs".

### Takeaways

After completing this assignment, you should:

* Understand process execution: forking, executing, and reaping.
* Understand signal handling.
* Understand the use of "dup" to perform I/O redirection.
* Have a more advanced understanding of Unix commands and the command line.
* Have gained experience with C libraries and system calls.
* Have enhanced your C programming abilities.

## Hints and Tips

* We **strongly recommend** that you check the return codes of **all** system calls
  and library functions.  This will help you catch errors.
* **BEAT UP YOUR OWN CODE!** Use a "monkey at a typewriter" approach to testing it
  and make sure that no sequence of operations, no matter how ridiculous it may
  seem, can crash the program.
* Your code should **NEVER** crash, and we will deduct points every time your
  program crashes during grading.  Especially make sure that you have avoided
  race conditions involving process termination and reaping that might result
  in "flaky" behavior.  If you notice odd behavior you don't understand:
  **INVESTIGATE**.
* You should use the `debug` macro provided to you in the base code.
  That way, when your program is compiled without `-DDEBUG`, all of your debugging
  output will vanish, preventing you from losing points due to superfluous output.

> :nerd: When writing your program, try to comment as much as possible and stay
> consistent with code formatting.  Keep your code organized, and don't be afraid
> to introduce new source files if/when appropriate.

### Reading Man Pages

This assignment will involve the use of many system calls and library functions
that you probably haven't used before.
As such, it is imperative that you become comfortable looking up function
specifications using the `man` command.

The `man` command stands for "manual" and takes the name of a function or command
(programs) as an argument.
For example, if I didn't know how the `fork(2)` system call worked, I would type
`man fork` into my terminal.
This would bring up the manual for the `fork(2)` system call.

> :nerd: Navigating through a man page once it is open can be weird if you're not
> familiar with these types of applications.
> To scroll up and down, you simply use the **up arrow key** and **down arrow key**
> or **j** and **k**, respectively.
> To exit the page, simply type **q**.
> That having been said, long `man` pages may look like a wall of text.
> So it's useful to be able to search through a page.
> This can be done by typing the **/** key, followed by your search phrase,
> and then hitting **enter**.
> Note that man pages are displayed with a program known as `less`.
> For more information about navigating the `man` pages with `less`,
> run `man less` in your terminal.

Now, you may have noticed the `2` in `fork(2)`.
This indicates the section in which the `man` page for `fork(2)` resides.
Here is a list of the `man` page sections and what they are for.

| Section          | Contents                                |
| ----------------:|:--------------------------------------- |
| 1                | User Commands (Programs)                |
| 2                | System Calls                            |
| 3                | C Library Functions                     |
| 4                | Devices and Special Files               |
| 5                | File Formats and Conventions            |
| 6                | Games, et al                            |
| 7                | Miscellanea                             |
| 8                | System Administration Tools and Daemons |

From the table above, we can see that `fork(2)` belongs to the system call section
of the `man` pages.
This is important because there are functions like `printf` which have multiple
entries in different sections of the `man` pages.
If you type `man printf` into your terminal, the `man` program will start looking
for that name starting from section 1.
If it can't find it, it'll go to section 2, then section 3 and so on.
However, there is actually a Bash user command called `printf`, so instead of getting
the `man` page for the `printf(3)` function which is located in `stdio.h`,
we get the `man` page for the Bash user command `printf(1)`.
If you specifically wanted the function from section 3 of the `man` pages,
you would enter `man 3 printf` into your terminal.

> :scream: Remember this: **`man` pages are your bread and butter**.
> Without them, you will have a very difficult time with this assignment.

## Getting Started

Fetch and merge the base code for `hw4` as described in `hw1`.
You can find it at this link: https://gitlab02.cs.stonybrook.edu/cse320/hw4

Here is the structure of the base code:
<pre>
.
├── .gitlab-ci.yml
└── hw4
    ├── demo
    │   └── mush
    ├── include
    │   ├── debug.h
    │   ├── mush.h
    │   ├── mush.tab.h
    │   └── syntax.h
    ├── Makefile
    ├── rsrc
    │   ├── bg_test.mush
    │   ├── cancel_test.mush
    │   ├── delete_test.mush
    │   ├── fg_test.mush
    │   ├── goto_test.mush
    │   ├── list_test.mush
    │   ├── loop1.mush
    │   ├── loop2.mush
    │   ├── pause_test.mush
    │   ├── pipeline_test.mush
    │   ├── run_test.mush
    │   ├── stop_test.mush
    │   └── wait_test.mush
    ├── src
    │   ├── execution.c
    │   ├── jobs.c
    │   ├── main.c
    │   ├── mush.lex.c
    │   ├── mush.tab.c
    │   ├── program.c
    │   ├── store.c
    │   └── syntax.c
    └── tests
        └── base_tests.c
</pre>

If you run `make`, the code should compile correctly, resulting in an
executable `bin/mush`.  If you run this program, it doesn't do very
much, because there are a number of pieces that you have to fill in.

## `Mush`: Overview

The `mush` language is a simple programming language which was roughly
inspired by the classic programming language BASIC.
A `mush` program consists of a set of *statements*, with one statement
per line of program text.
The syntax of statements is given by the following context-free grammar:

```
<statement>       ::= list
                    | delete <lineno> , <lineno>
                    | run
                    | cont
                    | <lineno> stop
                    | <optional_lineno> set <name> = <expr>
                    | <optional_lineno> unset <name>
                    | <optional_lineno> goto <lineno>
                    | <optional_lineno> if <expr> goto <lineno>
                    | <optional_lineno> source <file>
                    | <optional_lineno> <pipeline>
                    | <optional_lineno> <pipeline> &
                    | <optional_lineno> wait <expr>
                    | <optional_lineno> poll <expr>
                    | <optional_lineno> cancel <expr>
                    | <optional_lineno> pause
```

Some kinds of statements have required *line numbers*, other kinds of
statements have no line numbers, and for some statements the line numbers
are optional.  In general, when the `mush` interpreter reads a statement
without a line number, it is executed immediately, whereas when it reads
a statement with a line number it is not immediately executed, but instead
is saved in the *program store*.
The program store maintains a set of statements, each of which has a
line number.  In addition, the program store maintains a *program counter*,
which keeps track of the next statement to be executed when `mush` is
in "run mode".

The `list`, `delete`, `run`, and `cont` statements have no line numbers,
and so can only be executed immediately.  The `list` statement causes
`mush` to list the contents of the program store.  The `delete` statement
deletes statements from the program store whose line numbers lie within
a specified range.  The `run` statement causes `mush` to reset the program
counter to the lowest-numbered statement in the program store and to begin
running automatically.  The `cont` statement causes `mush` to continue
automatic execution that has been stopped by the execution of a `stop` statement.
Since a `stop` statement has a required line number, such a statement
can never be executed immediately, but rather only from the program store
during automatic execution.

The remaining statements have optional line numbers, and so can be executed
either immediately or from the program store.
The `set` statement is used to set the value of a variable to be the result
of evaluating an expression.
The `unset` statement is used to un-set the value of a variable, leaving it
with no value.
The `goto` statement resets the program counter so that the next statement to
be executed is the one with the specified line number.
The `if` statement causes control to be transferred conditionally to the
statement with the specified line number, if the specified expression evaluates
to a non-zero number.
The `source` statement causes `mush` to interpret the statements in the specified
file before continuing on with the current program.

A statement can also consist of a *pipeline*, to be executed either in the
"foreground" or in the "background".  A pipeline consists of a sequence of
*commands*, separated by vertical bars (`|`), with possible
*input redirection*, specified using `<` followed by a filename,
*output redirection*, specified using `>` followed by a filename,
or *output capturing*, specified using `>@`.
A pipeline is executed by `mush` in much the same fashion as it would be
executed by a shell such as `bash`: a group of processes is created to run
the commands concurrently, with the output of each command in the pipeline
redirected to become the input of the next command in the pipeline.
If input redirection is specified, then the first command in the pipeline
has its input redirected from the specified file.
If output redirection is specified, then the last command in the pipeline
has its output redirected to the specified file.
If output capturing is specified, then the output of the last command in the
pipeline is read by the `mush` interpreter itself, which makes it available
as the value of a variable that can be referenced by the execution of
subsequent statements in the program.

Each command in a pipeline consists of a nonempty sequence of *args*,
where the first arg in the command specifies the name of a program to be run
and the remaining args are supplied to the program as part of its argument
vector.  In `mush`, an arg takes the form of an *atomic expression*,
which can be either a *string variable*, a *numeric variable*,
a *string literal*, or an arbitrary expression enclosed in parentheses.

The syntax of pipelines, commands, and args is given by the following grammar:

```
<pipeline>        ::= <command_list>
                    | <pipeline>  <  <file>
                    | <pipeline>  >  <file>
                    | <pipeline>  >@

<command_list>    ::= <command>
                    | <command>  |  <command_list>

<command>         ::= <arg_list>

<arg_list>        ::= <arg>
                    | <arg> <arg_list>

<arg>             ::= <atomic_expr>

<atomic_expr>     ::= <literal_string>
                    | <numeric_var>
                    | <string_var>
                    | ( <expr> )
```

`Mush` supports *expressions* built up from *string variables*,
*numeric variables*, and *literal strings*, using various unary
and binary operators, as given by the following grammar:

```
<expr>            ::= <atomic_expr>
                    | <expr>  ==  <expr>
                    | <expr>  <  <expr>
                    | <expr>  >  <expr>
                    | <expr>  <=  <expr>
                    | <expr>  >=  <expr>
                    | <expr>  &&  <expr>
                    | <expr>  ||  <expr>
                    |  !  <expr>
                    | <expr>  +  <expr>
                    | <expr>  -  <expr>
                    | <expr>  *  <expr>
                    | <expr>  /  <expr>
                    | <expr>  %  <expr>
```

A *string variable* consists of a `$` symbol followed by a *name*,
which is a sequence of alphanumeric characters and underscores,
beginning with an alphabetic character or an underscore.
A *numeric variable* is similar, except it uses a `#` symbol in place
of the `$`.
A *literal string* is either a *number*, which consists of digits,
a *word*, which consists of non-whitespace characters which do not otherwise
have some special meaning to `mush`, or a *quoted string*, which is enclosed
in double quotes and which may contain special characters.
A *filename* that appears in the input or output redirection part of a
pipeline is permitted to be either a word or a quoted string.
This allows simple filenames without special characters to be specified
without quotes.  Filenames that contain special characters (including `/`)
must be specified as quoted strings.

Here is a simple example of a `mush` program:

```
10 echo "Let's start!"
20 set x = 0
30 date >@
40 set d = $OUTPUT
50 echo The date and time is: $d
60 sleep 1
70 set x = #x + 1
80 if #x <= 10 goto 30
90 stop
```

The remaining types of statements that `mush` understands have to do with
the manipulation of concurrently executing *jobs*.
Each time `mush` executes a pipeline statement, a new job is created.
`Mush` keeps track of the existing jobs in a *jobs table*.
Each job in the jobs table has an associated *job ID*, which is a nonnegative
integer that uniquely identifies the job.
After starting a job, `mush` sets the value of the `JOB` variable to be
the job ID of the job that was started.
For a foreground job, `mush` waits for the job to complete and then sets the
value of the `STATUS` variable to be the exit status of the job.
`Mush` then *expunges* the job from the jobs table.
For a background job, `mush` does not wait for the job to complete, but instead
continues execution.  At a later time, a `wait` statement can be executed
in order to wait for the background job to complete, to collect its
exit status, and to expunge the job.  Alternatively, a `poll` statement can
be executed to check whether the job has terminated without waiting if it
has not.  If the job has terminated, then the exit status is collected and
the job is expunged with a `poll` statement, similarly to a `wait` statement.
Execution of a `cancel` statement makes an attempt to cancel a specified
background job.  A `SIGKILL` signal is sent to the process group to which the
processes in the jobs belong.  If the processes have not already terminated,
then they will terminate upon receiving the `SIGKILL` signal.
A `wait` statement may be used to wait for this termination to occur and
to expunge the canceled job from the jobs table.
Note that the `wait`, `poll`, and `cancel` statements all permit the use of an
arbitrary expression to specify the job ID.

The final kind of statement that `mush` supports is the `pause` statement.
This statement causes execution to be suspended pending the receipt of a signal
that might indicate a change in the status of jobs in the jobs table.
When such a signal is received, execution continues.
This way, `mush` can wait for a change in job status without consuming an
excessive amount of CPU time.

### Demonstration version

To help you understand how `mush` is intended to behave, I have provided a
demonstration version as a binary with the assignment basecode.
This can be found as the executable `demo/mush`.
This demonstration version is intended as an aid to understanding only;
it should not be regarded as a specification of what you are to do.
It is likely that the demonstration version has some bugs or that its
behavior does not conform in some respects to what is stated here and in
the specifications in the basecode.

## Tasks to be Completed

Included in the basecode for this assignment is an implementation of a
parser for `mush` statements and the basic control structure of the
`mush` interpreter.  A number of modules have been left for you to
implement.  These are:

 * A *program store* module, which is used to hold a `mush` program
   and manage the program counter.

 * A *data store* module, which is used to keep track of the current values
   of the variables used in a `mush` program.

 * A *jobs* module, which keeps track of the currently executing jobs using
   a jobs table, and implements job manipulation functions used to execute
   and wait for pipelines, collect exit status, perform input and output
   redirection, and implement the output capture feature of `mush`.

### The Program Store Module

Specifications and stubs for the functions that make up the program store module
of `mush` are given in the source file `src/program.c`.
Implementation of these functions from the specifications should be relatively
straightforward, so I will not spend additional space on them here.
The choice of data structure used to represent the program store has been left
to you.
Pay close attention to what the specifications say about who has the responsibility
for freeing the memory associated with statements in the store.
A correct implementation should not leak memory associated with program statements,
and of course it should not suffer from double free bugs and the like.

### The Data Store Module

Specifications and stubs for the functions that make up the data store module
of `mush` are given in the source file `src/store.c`.
Once again, I expect that implementation of these functions should be relatively
straightforward.  As for the program store, the choice of data structure used
to implement the data store is for you to make and you should pay attention to
what the specifications say about who is responsible for freeing memory.

### The Jobs Module

Specifications and stubs for the functions that make up the jobs module
of `mush` are given in the source file `src/jobs.c`.
It is this module that is likely to be unfamiliar and to present some challenges
to you, so I am providing some additional guidance here.

 * You will need to implement some form of "jobs table" in this module,
   to keep track of the jobs that have been created but not yet expunged.
   The data structure you use is up to you.  If you find it convenient,
   you may assume that at most `JOBS_MAX` jobs can exist at one time,
   where `JOBS_MAX` is a C preprocessor symbol defined in `mush.h`.
   Write your code so that it does not depend on a particular value for
   `JOBS_MAX`; do not hard-code the value into your implementation.

 * Your jobs module will need to make use of handlers for two types of signals.
   The first is the `SIGCHLD` signal used to obtain notifications when a child
   process terminates.  This has been discussed in class and can also be found
   in the textbook.
   The second type of signal you will need to handle is the `SIGIO` signal used
   to obtain notifications when a file descriptor is ready for reading.
   This will be important to enable your program to capture output from
   concurrently executing background jobs without the need to commit to waiting
   for data from any one of them at any particular time.  This is discussed
   further below.
   
 * For correct operation, your implementation will likely have to make use of
   the `sigprocmask()` function to mask signals during times when a signal handler
   should be prevented from running.  You will likely also need to use the
   `sigsuspend()` function under certain circumstances to await the arrival of a
   signal.

 * When executing a pipeline consisting of N commands, a total of N+1 processes
   should be used.  One of these processes, which we will call the pipeline
   *leader*, should be the direct child of the main `mush` process.
   The remaining `N` processes will be children of the leader process, and will
   each execute one of the commands in the pipeline.
   The leader process should set itself into a new process group using its own
   process ID as the process group ID, and its `N` child processes should belong
   to this process group.  This is so that job cancellation can be performed by
   sending just one `SIGKILL`, directed at the process group for the job.
   The leader process should wait for and reap its `N` children before terminating.
   The main `mush` process should use its `SIGCHLD` handler to receive notifications
   about the termination of pipeline leader processes and to collect their
   exit status.

 * Besides the `fork()` system call used to create the processes, the creation of the pipeline
   will involve the use of the `open()`, `pipe()`, and `dup2()` system calls to set up the pipes
   and redirections, and the `execvp()` system call must be used to execute the individual
   commands.

  > **Important:**  You **must** create the processes in a pipeline using calls to
  > `fork()` and `execvp()`.  You **must not** use the `system()` function, nor use any
  > form of shell in order to create the pipeline, as the purpose of the assignment is
  > to give you experience with using the system calls involved in doing this.

 * Once having set up the pipeline, the pipeline leader will use `wait()` or `waitpid()`
   to await the completion of the processes in the pipeline.
   The leader process should wait for all of its children to terminate before
   terminating itself.  The leader should return the exit status of the process
   running the last command in the pipeline as its own exit status, if that
   process terminated normally.  If the last process terminated with a signal,
   then the leader should terminate via SIGABRT.

 * The `pipe()` and `dup2()` system calls should be used to perform the input
   and output redirection associated with a pipeline, as discussed in class and
   in the textbook.  Files used for input and output redirection should be opened
   using the `open()` system call.  For correct operation of a pipeline, care
   should be taken while setting up the pipeline that each process makes sure to
   `close()` pipe file descriptors that it does not use.

 * The capturing of output from a pipeline by the main `mush` process is to be
   accomplished as follows.  Before forking the pipeline leader, a pipe should
   be created to provide a way to redirect output from the last process in the
   pipeline back to the main `mush` process.  The redirection will be accomplished
   using `dup2()` as usual.  The main `mush` process will need to save the file
   descriptor for the read side of the pipe in the jobs table along with other
   state information from that job.  Output from the pipeline will be collected
   by the main `mush` process by reading from the read side of the pipe and
   saving what is read in memory.  Automatic dynamic allocation of however much
   memory is required to hold the output can be accomplished by using the
   `open_memstream()` function to obtain a `FILE` object to which the data can
   be written.

   The main technical issue involved in output capturing is how to arrange for
   the main `mush` process to collect the output produced from multiple
   concurrently executing pipelines, without having to block waiting for any one
   of them to produce output at any given time.  This can be done using so-called
   *asynchronous I/O*.  When the main `mush` process creates the pipe from which
   it will read the captured data, it should perform the following system calls
   (`readfd` is the file descriptor for the read side of the pipe):
   
   ```
      fcntl(readfd, F_SETFL, O_NONBLOCK);
      fcntl(readfd, F_SETFL, O_ASYNC);
      fcntl(readfd, F_SETOWN, getpid());
   ```

   The first of these calls enables *non-blocking I/O* on the file descriptor.
   This means that an attempt to `read()` the file descriptor when no data is
   available will not cause the main `mush` process to block (*i.e.* wait for
   data to arrive); rather the `read()` will return immediately with an error
   and `errno` set to `EWOULDBLK`.
   The second call sets *asynchronous mode* on the file descriptor.
   When this is set, the operating system kernel will send a `SIGIO` signal
   whenever there has been a change in status of the file descriptor; for example,
   whenever data becomes available for reading.
   The third call is necessary to set the "ownership" of the file descriptor
   to the main `mush` process, so that the kernel knows to which process
   the `SIGIO` signals should be directed.

   Once you have done this, then the main `mush` process can use a handler for
   `SIGIO` signals to become notified when there is output that needs to be
   captured.  It can then poll each of the file descriptors from which output
   is supposed to be captured, using `read()` to read input from each of them
   and save it in memory, until `EWOULDBLK` indicates that there is no more data
   currently available.  This way, it can collect the captured output in a timely
   fashion without getting "stuck" waiting for output that might take an
   indefinite amount of time to arrive.

   For more information, you will have to look at the man pages for the various
   system calls involved, including `pipe()`, `dup2()`, `fcntl()`, `open()`, `read()`,
   `signal()` (or `sigaction()`), `sigprocmask()`, and `sigsuspend()`.

## Using `gdb` to Debug Multi-process Programs

Although it gets harder to debug using `gdb` once multiple processes are involved,
there is some support for it.  The `gdb` command `set follow-fork-mode parent`
causes `gdb` to follow the parent process after a `fork()` (this is the default).
Similarly, the command `set follow-fork-mode child` causes `gdb` to follow the child
process instead.

## Provided Components

### The `mush.h` Header File

The `mush.h` header file that we have provided gives function prototypes for
the functions that you are to implement, and contains a few other related
definitions.  The actual specifications for the functions will be found
as comments attached to stubs for these functions in the various C source files.

  > :scream: **Do not make any changes to `mush.h`.  It will be replaced
  > during grading, and if you change it, you will get a zero!**

### The `syntax.h` Header File

The `syntax.h` header file that we have provided defines the data structures
used to represent parsed `mush` statements.  Mostly, you don't have to know
much about the details of these data structures, except, for example,
that you will need to be able to extract some information from them,
such as the pipeline from a foreground or background pipeline statement.
To avoid memory leaks, you will need to use the various `free_xxx()`
functions provided to free syntactic objects when they are no longer being used.
You will also need to use the function provided to make a copy of a pipeline
object in a certain situation -- see the specification for `jobs_run()` for
more information.

  > :scream: **Do not make any changes to `syntax.h`.  It will be replaced
  > during grading, and if you change it, you will get a zero!**

### The `syntax.c` Source File

The `syntax.c` source file that we have provided contains the implementations
of the various functions for which prototypes are given in `syntax.h`.

  > :scream: **Do not make any changes to `syntax.c`.  It will be replaced
  > during grading, and if you change it, you will get a zero!**

### The `mush.lex.c`, `mush.tab.c`, and `mush.tab.h` Files

The basecode provides a parser for the `mush` language.  This parser is
implemented using the GNU `bison` parser generator. and the GNU `flex`
lexical analyzer generator.  The `mush.lex.c`, `mush.tab.c`, and `mush.tab.h`
files are auto-generated files produced by the `bison` and `flex` programs.

  > :scream: **None of these files should be changed or edited.
  > Do *not* do the sloppy things that lots of people seem to do,
  > namely, editing these files, reformatting them or otherwise mutating them,
  > and then committing the changed results to `git`.  You will regret it
  > if you do this, and you have been duly warned!**

### The `demo/mush` Executable

The file `demo/mush` is an executable program that behaves more or less like
how your program should behave when it is finished.

  > :scream:  The behavior of the demo program should be regarded as an example
  > implementation only, not a specification.  If there should be any discrepancy
  > between the behavior of the demo program and what it says either in this document
  > or in the specifications in the header files, the latter should be regarded
  > as authoritative.

### The `rsrc` Directory

The `rsrc` directory contains some sample `mush` scripts which I used while
writing the demo version.  They were mostly designed very quickly to exercise
the basic features of `mush`, to verify that they worked to a first cut.
One way to run them is to type *e.g.* `source rsrc/xxx_test.mush` to the
`mush` prompt, to get it to read and execute the test.
If you have run one test and you want to run another, you should use the
`delete` command to clear any statements from the program store that might
have been left by the first test, otherwise they might interfere with the
new test.

### The `tests` Directory

The `tests` directory contains just one file, `base_tests.c`, which contains one
Criterion test that isn't very interesting.  This file is basically just a
placeholder where you can put tests you might think of yourself.

## Hand-in instructions
As usual, make sure your homework compiles before submitting.
Test it carefully to be sure that doesn't crash or exhibit "flaky" behavior
due to race conditions.
Use `valgrind` to check for memory errors and leaks.
Besides `--leak-check=full`, also use the option `--track-fds=yes`
to check whether your program is leaking file descriptors because
they haven't been properly closed.
You might also want to look into the `valgrind` `--trace-children` and related
options.

Submit your work using `git submit` as usual.
This homework's tag is: `hw4`.
