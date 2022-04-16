# CSE 320 Reference

**NOTE: This document has traditionally been provided (in PDF form) at the beginning
of the course; however, it was written in the ancient past and the source was no longer
available.  This version (in Markdown) has been reverse-engineered from the PDF source,
so that it can be updated in the future.  The reverse engineering turned up some errors
in the original document, and it likely introduced new errors.  But now the errors can
be corrected if somebody reports them :smiley:.**

## Using the Terminal

Great resources for understanding and working with command line:

[http://www.ibm.com/developerworks/library/l-lpic1-103-1/](http://www.ibm.com/developerworks/library/l-lpic1-103-1/)

[https://learnpythonthehardway.org/book/appendixa.html](https://learnpythonthehardway.org/book/appendixa.html)

## GCC

```c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	printf("Hello World!\n");
	return EXIT_SUCCESS;
}
```

### Lines 1 and 2

Lines 1 and 2 are the C **preprocessor** statements which include
**function prototypes** for some of the functions in the **C standard library**
(aka libc). For now you can just vaguely relate these to the `import`
statements you might find atthe top of a java file.

```java
import java.util.scanner;
```

The C preprocessor is a very powerful tool and you will learn about it
in future assignments. For now, just accept this basic explanation of
what these two lines do. The `#include` directive takes the contents of
the `.h` file and copies it into the `.c` file before the C compiler
actually translates the C code.

> :nerd: Files that end in .h are called header files. They typically
  contain preprocessor macros,function prototypes, **struct information**,
  and **typedefs**.

### Line 4

Line 4 is how you describe the `main()` function of a C program. In C,
if you are creating an executable program it must have one and ONLY one
main function. It should also be as isolated as possible, if you can
(and for this class you should always) have `main()` in its own `.c`
file. Any main function you write in this course MUST return an integer
value (in older textbooks/documentation they might return `void`; watch
out).

This is sort of similar to the `main()` declaration in Java. In Java,
arrays, since they are objects, have various different attributes (*e.g.*
length). C is not an object oriented language and hence arrays contain
no such information (arrays in C are very similar to arrays in
MIPS). To remedy this issue two arguments are passed: `argc`,
which contains how many elements are in the array and `argv`, which is an
array of strings which contains each of the arguments passed on the
command line. Even if no arguments are passed by the user, `argv` will
contain at least one argument which is the name of the binary being
executed.

> :nerd: If you look through other C programs, you might see that
  there are quite a few different ways to declare `main`. In this course
  you may declare `main` just as it is in the `helloworld` example unless
  specified otherwise in the homework assignment.

> :scream: It is crucial that there exists exactly one `main()` function
  in your whole program. C is not like Java, where you can have a
  different main in every file and then choose which main you want to
  run. If you have more than one main when you try to compile it will
  give you an error. For example, assume you had two files `main1.c` and
  `main2.c` and you tried to compile them both into one program
  (reasonable thing to do). If both, `main1.c` and `main2.c`, have a main
  function defined in them, when you try to compile it you get the
  following linker error:

  ```
  /tmp/cc8eYGEA.o: In function ‘main’:
  main2.c:(.text+0x0): multiple definition of ‘main’
  /tmp/ccaaqneq.o:main1.c:(.text+0x0): first defined here
  collect2: error: ld returned 1 exit status
  ```

  This error means that the main function is defined twice within your
  program. This concept extends to all functions. Two functions *CAN NOT*
  have the same name under normal conditions. In addition, function
  overloading is not allowed in C. Example: Assume you had the file
  func.c with the following function declarations.

  ```c
  void func(int a);
  void func(int a, int b);
  ```

  This will result in the following error

  ```
  func.c:5:6:error: conflicting types for ‘func’
  void func(int a, int b) {
       ^
  func.c:1:6: note: previous definition of ‘func’ was here
  void func(int a) {
  ```

### Line 5

Line 5 is how this program is printing out its values to standard
output (stdout). The printf function can be compared to the
System.out.printf() function in Java. This function accepts a char*
argument known as the format string (assume for now char* is equivalent
to the Java String type). This will work fine for when you know ahead
of time what you want to print, but what if you want to print a
variable?

If you assume C is like Java, you may try to concatenate strings in
the following form:

```java
int i = 5;
printf("The value of i is " + i + "\n");
```

If you try to compile this code, GCC may give you some of the
following cryptic error messages:

```
error: invalid operands to binary + (have ‘char *’ and ‘char *’)
```

or

```
warning: format not a string literal and no format arguments [-Wformat-security]
```

Unfortunately C, does not have string concatenation via the +
operator. However, the `printf()` function also takes a variable number
of arguments after the format string. In order to print a variable you
have to specify one of many available **conversion specifiers**
(character(s) followed by a % sign). Below is an example of how to
print an integer in C.

> :nerd: You can view a list of all printf formats here. Alternatively
  you can use the command `man 3 printf` in your terminal to view the
  documentation for printf as well. This is an example of a man
  page (manual page). Man pages are how most of the library functions in
  C are documented. You are highly encouraged to utilize them as they are
  extremely useful and highly beneficial. Man pages are also available
  online.

The printf function always prints to the filestream known as `stdout`
(standard output). There are three **standard streams** that are usually
available to each program, namely: `stdin` (standard input), `stdout`, and
`stderr` (standard error). Prior to `*nix`, computer programs needed to
specify and be connected to a particular I/O device such as magnetic
tapes. This made portability nearly impossible. Later in the course we
will delve deeper into “files” and how they represent abstract devices
in Unix-like operating systems. For now understand that they work
muchlike your typical .txt file. They can written to and read from.

### Line 6

Line 6 is the end of the main function. The value returned in main is
the value that represents the return code of the program. In `*nix` when
a program exits successfully, the value returned is usually zero. When
it has some sort of an error, the value is usually a non-zero
number. Since these values are defined by programmers and they may
be different depending on the system you are using, it is usually best
to use the constants `EXIT_SUCCESS` and `EXIT_FAILURE` which are defined in
`stdlib.h` for simple cases as they will represent the respective exit
codes for each system.

> The term `*nix` is used for describing operating systems that are
  derived from the *Unix* operating system (ex. BSD, Solaris) or clones of
  it (ex. Linux).

## Compiling C Code

Begin compiling the following program:

```c
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char* argv[]) {
	printf("Hello World!\n");
	return EXIT_SUCCESS;
}
```

Navigate on the command line to where the `.c` file is located. If the
file was called `helloworld.c`, type the following command to compile the
program.

```
$ gcc helloworld.c
```

> The `$` is the commandline prompt. **Your prompt may differ**.

If no messages print, that means there were no errors and the
executable was produced. To double check that your program produced a
binary you can type the `ls` command to list all items in the directory.

```
$ ls
a.out helloworld.c
$
```

The file **`a.out`** is your executable program. To run this program,
put a `./` in front of the binary name.

```
$ ./a.out
Hello World!
$
```

> The `./` has a special meaning. The `.` translates to the path of the
current directory. So if your file was in the cse320 directory on the
user’s desktop then when you type `./a.out` this would really
translate to the path `/home/user/Desktop/cse320/a.out`.

## Compilation Flags

Modify the `helloworld` program to sum up the values from 0 to 5.

```c
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {
	int i, sum;
	for(i = 0; i < 6; i++) {
		sum += i;
	}
	printf("The sum of all integers from 0-5 is: %d\n", sum);
	return EXIT_SUCCESS;
}
```

Compile and run this program.

```
$ gcc helloworld2.c
$ ./a.out

The sum of all integers from 0-5 is: 15
$
```

This program compiled with no errors and even produced the correct
result. However, there is a subtle but hazardous bug in this code. The
developers of the **gcc C compiler** have built in some functionalities
(enabled by flags) to help programmers find them.

Add the flags `-Wall` and `-Werror` to the `gcc` command when compiling. As so:

```
$ gcc -Wall -Werror helloworld2.c
helloworld2.c:7:3: error: variable 'sum' is uninitialized when used here
        [-Werror,-Wuninitialized]
    sum += i;
    ^~~
helloworld2.c:5:12: note: initialize the variable 'sum' to silence this warning
    int i, sum;
              ^
               = 0
1 error generated.
$
```

> Depending on your compiler (gcc, clang, etc.) the above error and
  message may differ.  Recent versions of gcc only produce an error when
  optimization (`-O1`, `-O2`, or `-O3`) is enabled.

> The flag `-Wall` enables warnings for all constructions that some users
  consider questionable, and that are easy to avoid (or modify to prevent
  the warning), even in conjunction with macros.

> The flag `-Werror` converts all warnings to errors. Source code
> which triggers warnings will be rejected.

This error means that the variable `sum` was used without being
initialized. Why does this matter? The C language does not actually
specify how the compiler should treat uninitialized
variables. Implementations of the C compiler may zero them out for you,
but really there is no specification of how this situation should be
handled. This can lead to undefined behavior and cause the program to
work one way one system and differently on other systems. To fix this
error, simply initialize the variable sum to the value desired (0).

```c
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {
	int i, sum = 0;
	for(i = 0; i < 6; i++) {
		sum += i;
	}
	printf("The sum of all integers from 0-5 is: %d\n", sum);
	return EXIT_SUCCESS;
}
```

Compile the program again and you should no longer see any errors.

```
$ gcc -Wall -Werror helloworld2.c
$ ./a.out
The sum of all integers from 0-5 is: 15
$
```

> :scream: In this class, you *MUST ALWAYS* compile your assignments
> with the flags `-Wall -Werror`. This will help you locate mistakes in
> your program and the grader will compile your assignment withthese
> flags as well. Consider this your warning, `-Wall -Werror` are
> necessary. Do not progress through your assignment without using
> these flags and attempt to fix the errors they highlight last minute.

## GNU Make and Makefiles

As you program more in C, you will continue to add more flags and more
files to your programs. To type these commands over and over again will
eventually become an error laden chore. Also as you add more files, if
you rebuild every file every time, even if it didn’t change, it will
take a long time to compile your program. To help alleviate this issue
build tools were created. One such tool is GNU Make (you will be
required to use Make in this class). Make itself has lots of options
and features that can be configured. While mastering Make is not
required from this class, you will probably want to learn how to make
simple changes to what we supply.

Refer
[here](http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/)
for a great Makefile tutorial and information resource. **You will
always be provided with a working makefile, this is provided for
extended learning.**

[http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/](http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/)

## Header Files

There are some coding practices that you should become familiar with
in C from the beginning. The C compiler reads through your code once
and only once. This means all functions and variables you use must be
declared in advance of their usage or the compiler will not know how to
compile and exit with errors. This is why we have header files, we
declare all of our function prototypes in a `.h` file and
`#include` it in our `.c` file. This is so we can write the body of our
functions in any order and call them in any order we please.

A header file is just a file which ends in the `.h` extension. Typically
you declare **function prototypes**, define `struct` and `union` types,
`#include` other header files, `#define` constants and macros, and
`typedef`. Some header files also expose global variables, but this is
strongly discouraged as it can cause compilation errors.

When you define function prototypes in a `.h` file, you can then define
the body of the function inside of any `.c` file. Though typically, if
the header file was `called example.h`, we would define the functions in
`example.c`. If we were producing a massive library like
[stdlibc](https://en.wikipedia.org/wiki/C_standard_library), you
may instead declare all the function prototypes in a single header file
but put each function definition in its own file. It’s all
a preference, but these are two common practices. You should never be
defining function bodies in the header though, this will just cause you
issues later.

There are two ways to specify where the include directive looks for
header files. If you use `<>`, when the preprocessor encounters the
include statement it will look for the file in a predefined location
on your system (usually `/usr/include`). If you use `""`, the preprocessor
will look in the current directory of the file being
processed. Typically system and library headers are included using `<>`,
and custom headers that you have made for your program are included
using `""`.

### Header file example

```c
#include<stdio.h>
#include<stdlib.h>
#include<stdlib.h>

#define TRUE 1
#define FALSE 0

struct student {
	char *first_name;
	char *last_name;
	int age;
	float gpa;
};

int foo(int a, int b);
void bar(void);
```


```c
#include"example.h"

int main(int argc, char *argv[]){
	bar();
	return EXIT_SUCCESS;
}

void bar(void){
	printf("foo: %d", foo(2, 3));
}

int foo(int a, int b) {
	return a * b;
}
```

### Header Guard a.k.a Include Guard

While using header files solves one issue, they create issues of their
own. What if multiple files include the same header file? What if
header file A includes header file B, and header file B includes
header file A? If we keep including the same header file multiple
times, this will make our source files larger than needed and slow
down the compilation process. It may also cause errors if there are
variables declared in the code. If two files keep including each other
how does the compiler know when to stop? To prevent such errors one
must utilize **header guards**. The header guard is used to prevent double
and cyclic inclusion of a header file.

### Header Guard example

In grandparent.h:

```c
struct foo {
    int member;
};
```

In parent.h:

```c
#include "grandparent.h"
```

In child.h:

```c
#include "grandparent.h"
#include "parent.h"
```

The linker will create a temporary file that has literal copies of the
`foo` definition twice and this will create a compiler error since the
compiler does not know which definition takes precedence. The fix:

In grandparent.h:

```c
#ifndef GRANDFATHER_H
#define GRANDFATHER_H
struct foo {
    int member;
};
#endif
```

In parent.h:

```c
#include "grandparent.h"
```

In child.h:

```c
#include "grandparent.h"
#include "parent.h"
```

`ifndef`, `#define`, `#endif` are preprocessor macros that
prevent the double inclusion. This is because when the `father.h` file
includes `grandfather.h` for the second time the `#ifndef` macro returns
false so the second definition for `foo` is never included.
Read [here](https://en.wikipedia.org/wiki/Include_guard#Double_inclusion)
for more information.

> You should always use header files and guards in your
  assignments.  Newer compilers now support what is known as `#pragma once`.
  This directive performs the same operation as the header guard,
  but it may not be a cross platform solution when considering
  older machines.

### Directory Structure

To help with a clear and consistent structure to your programs, you
can use the following directory structure. This is a common directory
structure for projects in C.

```
.
├── Makefile
├── include
│   ├── debug.h
│   └── func.h
└── src
    ├── main.c
    └── func.c
```

> :scream: You will be **REQUIRED** to follow this structure for **ALL** the homework
  assignments for this class. Failure to do so will result in a ZERO.

## Datatype Sizes

Depending on the system and the underlying architecture, which can
have different word sizes etc., datatypes can have various different
sizes. In a language like Java, much of these issues are hidden from
the programmer. The JVM creates another layer of abstraction which can
allow the programmer to believe all datatypes are of same size no
matter the underlying architecture. C, on the other hand, does not
have this luxury. The programmer has to consider everything about the
system being worked on. To make programs cross platform, code and
logic needs to be tested, comparing results and output, and altered
accordingly.

C lacks the ability to add new datatypes to its
specification. Instead, it works with models known as LP64,
ILP64, LLP64, ILP32, and LP32. The `I` stands for `INT`, the `L` stands for
`LONG` and the `P` stands for `POINTER`. The number after the letters
describes the maximum bit size of the data types.

The typical sizes of these models are described below in the following
table (in bits):

```
TABLE WAS MISSING IN ORIGINAL -- NEED TO RECONSTRUCT!
```

Notice that the size of an integer on one machine could be different
from that on another machine depending on which model the machine
runs. To prove this to yourself, use the special operator in the C
language known as `sizeof`. The operator `sizeof` will tell you the size of
a specific datatype in bytes. As an exercise, you should create the
following program and run it in your development environment and on
a system with a different underlying architecture (such as 'Sparky')
and compare the results.

```c
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	/* Basic data types */
	printf("=== Basic Data Types ===\n");
	printf("short: %lu bytes\n", sizeof(short));
	printf("int: %lu bytes\n", sizeof(int));
	printf("long: %lu bytes\n", sizeof(long));
	printf("long long: %lu bytes\n", sizeof(long long));
	printf("char: %lu byte(s)\n", sizeof(char));
	printf("double: %lu bytes\n", sizeof(double));
	/* Pointers */ printf("=== Pointers ===\n");
	printf("char*: %lu bytes\n", sizeof(char*));
	printf("int*: %lu bytes\n", sizeof(int*));
	printf("long*: %lu bytes\n", sizeof(long*));
	printf("void*: %lu bytes\n", sizeof(void*));
	printf("double*: %lu bytes\n", sizeof(double*));
	/* Special value - This may have undefined results... why? */
	printf("=== Special Data Types ===\n");
	printf("void: %lu byte(s)\n", sizeof(void));
	return EXIT_SUCCESS;
}
```

To further illustrate why this is a problem, consider the following program.

```c
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	// 0x200000000 -> 8589934592 in decimal
	long value = strtol("200000000", NULL, 16);
	printf("value: %ld\n", value);
	return EXIT_SUCCESS;
}
```

In libc, there exists a header `stdint.h` which has special types
defined to make sure that if you use them, nomatter what system you
are on, it can guarantee that they are the correct size.

## Endianness

When dealing with multi byte values and different architectures, the
**endianness** of each architecture should also be taken into
account. There are many ways to detect what endianness your machine
is, for example:

```c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	unsigned int i = 1;
    char *c = (char*)&i; // Convert the LSB into a character
    if(*c) {
		printf("little endian\n");
    } else {
		printf("big endian\n");
    }
	return EXIT_SUCCESS;
}
```

Can you think of why this works? Could you explain it if asked on an exam?

## Assembly

During the compilation process, a C program is translated to an
assembly source file. This is important because it is possible that
something which has great performance in one system could have
terrible performance in another with the exact same C implementation,
in this case, the programmer has to inspect the assembly code for
more information.

Example:

```c
// asm.c
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
int main(int argc, char *argv[]) {
	char buffer[1024];
	// Get user input
	fgets(buffer, 1024, stdin);
	int64_t value = strtoll(buffer, NULL, 10);
	printf("You entered %" PRId64 "\n", value);
	return EXIT_SUCCESS;
}
```

Test the program with 32-bit binaries vs 64-bit binaries. To be able
to compile a 32-bit binary on a 64-bit machine, utilize the `-m32`
flag provided by gcc-multilib (installed during HW0). Here is how to
compile each program respectively:

```
$ gcc -Wall -Werror -m32 asm.c -o 32.out
$ gcc -Wall -Werror -m64 asm.c -o 64.out
```

Run each program and you should see this output:

```
$ ./64.out
75
You entered 75
$ ./32.out
75
You entered 75
```

	> 75 is a value that is entered by the user. You can enter any number you choose.

Notice, even though both programs are compiled for different
architectures, they still produce the same results.These programs are
assembled using different instruction sets though. To see this compile
the programs with the `-S` flag. This flag will store the intermediate
assembly of the program in a `.s` file.

For the 64-bit program run:

```
$ gcc -Wall -Werror -m64 -S asm.c
```

Take a look at `asm.s` which was just generated in the **current working directory**.

```
# x86-64 assembly for asm.c
	.file "asm.c"
	.section .rodata
.LC0:
	.string "You entered %ld\n"
	.text .globl main
	.type main, @function
main:
.LFB2:
	.cfi_startproc
	pushq %rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq %rsp, %rbp
	.cfi_def_cfa_register 6
	subq $1072, %rsp
	movl %edi, -1060(%rbp)
	movq %rsi, -1072(%rbp)
	movq %fs:40, %rax
	movq %rax, -8(%rbp)
	xorl %eax, %eax
	movq stdin(%rip), %rdx
	leaq -1040(%rbp), %rax
	movl $1024, %esi
	movq %rax, %rdi
	call fgets
	leaq -1040(%rbp), %rax
	movl $10, %edx
	movl $0, %esi
	movq %rax, %rdi
	call strtoll
	movq %rax, -1048(%rbp)
	movq -1048(%rbp), %rax
	movq %rax, %rsi
	movl $.LC0, %edi
	movl $0, %eax
	call printf
	movl $0, %eax
	movq -8(%rbp), %rcx
	xorq %fs:40, %rcx
	je .L3
	call __stack_chk_fail
.L3:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size main, .-main
	.ident "GCC: (Ubuntu 5.2.1-22ubuntu2) 5.2.1 20151010"
	.section .note.GNU-stack,"",@progbits
```

Now compile it for x86 using the following command:

```
$ gcc -Wall -Werror -m32 -S asm.c
```

Again, take a look at `asm.s` which was just generated in current working directory.

```
# x86 assembly for asm.c
	.file "asm.c"
	.section .rodata
.LC0:
	.string "You entered %lld\n"
	.text .globl main
	.type main, @function
main:.LFB2:
	.cfi_startproc
	leal 4(%esp), %ecx
	.cfi_def_cfa 1, 0
	andl $-16, %esp
	pushl -4(%ecx)
	pushl %ebp
	.cfi_escape 0x10,0x5,0x2,0x75,0
	movl %esp, %ebp
	pushl %ecx
	.cfi_escape 0xf,0x3,0x75,0x7c,0x6
	subl $1060, %esp
	movl %ecx, %eax
	movl 4(%eax), %eax
	movl %eax, -1052(%ebp)
	movl %gs:20, %eax
	movl %eax, -12(%ebp)
	xorl %eax, %eax
	movl stdin, %eax
	subl $4, %esp
	pushl %eax
	pushl $1024
	leal -1036(%ebp), %eax
	pushl %eax
	call fgets
	addl $16, %esp
	subl $4, %esp
	pushl $10
	pushl $0
	leal -1036(%ebp), %eax
	pushl %eax
	call strtoll
	addl $16, %esp
	movl %eax, -1048(%ebp)
	movl %edx, -1044(%ebp)
	subl $4, %esp
	pushl -1044(%ebp)
	pushl -1048(%ebp)
	pushl $.LC0
	call printf
	addl $16, %esp
	movl $0, %eax
	movl -12(%ebp), %edx
	xorl %gs:20, %edx
	je .L3
	call __stack_chk_fail
.L3:
	movl -4(%ebp), %ecx
	.cfi_def_cfa 1, 0
	leave
	.cfi_restore 5
	leal -4(%ecx), %esp
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE2:
	.size main, .-main
	.ident "GCC: (Ubuntu 5.2.1-22ubuntu2) 5.2.1 20151010"
	.section .note.GNU-stack,"",@progbits
```

Additionally you can log into sparky, and use the C compiler on that
machine. It will generate 32-bit SPARC assembly.

```
$ gcc -Wall -Werror -S asm.c
```

```
# 32-bit SPARC assembly
        .file   "asm.c"
        .section        ".rodata"
        .align 8
.LLC0:
        .asciz  "You entered %lld\n"
        .section        ".text"
        .align 4
        .global main
        .type   main, #function
        .proc   04
main:
        save    %sp, -1128, %sp
        st      %i0, [%fp+68]
        st      %i1, [%fp+72]
        add     %fp, -1032, %g1
        mov     %g1, %o0
        mov     1024, %o1
        sethi   %hi(__iob), %g1
        or      %g1, %lo(__iob), %o2
        call    fgets, 0
        nop
        add     %fp, -1032, %g1
        mov     %g1, %o0
        mov     0, %o1
        mov     10, %o2
        call    strtoll, 0
        nop
        std     %o0, [%fp-8]
        sethi   %hi(.LLC0), %g1
        or      %g1, %lo(.LLC0), %o0
        ld      [%fp-8], %o1
        ld      [%fp-4], %o2
        call    printf, 0
         nop
        mov     0, %g1
        mov     %g1, %i0
        return  %i7+8
         nop
        .size   main, .-main
        .ident  "GCC: (GNU) 4.9.1"
```

## Assembly Analysis

The assembly generated for a particular architecture varies greatly
even though it all accomplishes the exact same task on each
system. Notice that the SPARC assembly is shorter than the other two
(40 lines for SPARC, 67 lines for x86, and 51 lines for x86-64) and
that the registers used are different in all three examples.

Take a look at how the format string in the printf call got translated:

```c
printf("You entered %" PRId64 "\n", value);
```

```
.string "You entered %ld\n"  # x86-64; 64-bits
.string "You entered %lld\n" # x86; 32-bits
.asciz  "You entered %lld\n" # SPARC; 32-bits
```

See that PRId64 got translated to different formats: `%ld` and
`%lld`. This is because the `int64_t` is translated to different types
depending on the platform to guarantee that it is at least 64-bits
wide. In the SPARC code, notice thatthere are `nop` instructions after
the call to `printf`, `strtoll`, `fgets`, and return. This is because of a
technique known as **delayed branching** used in the SPARC architecture.

In the x86 assembly, notice `subl` and `pushl` instructions which are used
to manipulate the stack before calling functions. These instructions
are absent from the x86-64 example. This is because x86 architecture
has half the amount of registers as x86-64 architectures so the
convention is to push arguments for a function call to the stack
to compensate for this. At the core, the **Application Binary Interface**
differs between the systems. There are also various other differences
that can’t be seen by looking at the assembly such as variable sized
instruction formats, but, in general, you should just be aware that any
C code gets translated very differently depending on the machine.

## Preprocessor

Sometimes the easiest way to see what is happening in your program is
to just use print statements. This is a method that everyone can do
(and we know how to do!). However, we shouldn’t just put `printf` all
over our program. We do not always want to see these print outs (way
too much information for normal operation) and we don’t want to have to
comment/uncomment lines constantly.

One possible solution to this is passing a command line argument that
turns debugging on and off. This might be an acceptable solution but it
will clutter our code with lots of if statements to check if debugging
is enabled or not, make our binary larger when we don’t want debugging
enabled, etc. Instead we will use some preprocessor tricks to give us
some logging statements when we **compile with** the flag
`-DDEBUG`. When we **compile without** the flag `-DDEBUG`, none of these
debugging statements will be printed.

We have defined in the given Makefile a `debug` target. This compiles
your program with the `-DDEBUG` flag and `-g`, the latter of which is
necessary for gdb to work. You can simply run:

```
$ make clean debug
```

as opposed to `make clean all` to set your program up for debugging.

Create a new header called `debug.h` and we can define each of these
macros in this header and use them in `main()` by adding `#include "debug.h"`
to `main.c`.

debug.h:

```c
#ifndef DEBUG_H
#define DEBUG_H
#include<stdlib.h>
#include<stdio.h>

#define debug(msg) printf("DEBUG: %s", msg)

#endif
```

Then in your program use the debug macro

main.c:

```c
#include "debug.h"

int main(int argc, char *argv[]) {
	debug("Hello, World!\n");
	return EXIT_SUCCESS;
}
```

Then compile your program and run it.

```
$ make clean all
$ bin/hw1
DEBUG: Hello, World!
```

Great! You just created your first **preprocessor macro**. Unfortunately
this is no better than just adding a print statement. Let's fix that!

The preprocessor has `#if`, `#elif`, and `#else` **directives** that that we can
use to control what gets added during compilation. (Also `#endif` for
completing an if/else block) Let's create an *if* directive that will
include a section of code if `DEBUG` is defined within the preprocessor.

debug.h:

```c
#ifndef DEBUG_H
#define DEBUG_H
#include<stdlib.h>
#include<stdio.h>

#define debug(msg) printf("DEBUG: %s", msg)

#endif
```
main.c:

```c
#include "debug.h"

int main(int argc, char *argv[]) {
	#ifdef DEBUG
		debug("Debug flag was defined\n");
	#endif
	printf("Hello, World!\n");
	return EXIT_SUCCESS;
}
```

When we compile this program it will check to see if `#define DEBUG` was
defined in our program. Let's test this out.

```
$ make clean all
$ bin/hw1
Hello, World!
```

Cool the debug message didn’t print out. Now let's define `DEBUG` during
the compilation process, and run the program again.

```
$ make clean debug
$ bin/hw1
DEBUG: Debug flag was defined
Hello, World!
```

Here you can see that debug was defined so that extra code between
`#ifdef DEBUG` and `#endif` was included. This technique will work for
certain situations, but if we have a lot of logging messages in our
program this will quickly clutter our code and make it
unreadable. Fortunately we can do better.

Instead of doing `#ifdef DEBUG` all over our program we can instead do
`#ifdef DEBUG` around our `#define debug` macro.

debug.h:

```c
#ifndef DEBUG_H
#define DEBUG_H
#include<stdlib.h>
#include<stdio.h>

#if DEBUG
	#define debug(msg) printf("DEBUG: %s", msg)
#endif

#endif
```

main.c:

```c
#include"debug.h"

int main(int argc, char *argv[]) {
	debug("Debug flag was defined\n");
	printf("Hello, World!\n");
	return EXIT_SUCCESS;
}
```

There is an issue with this, but let's try to compile the program.

```
$ make clean debug
$ bin/hw1
DEBUG: Debug flag was defined
Hello, World!
```

Cool it works. Now let's try to compile it without defining `-DDEBUG`.

```
$ make clean all
/tmp/cc6F04VW.o: In function `main':
debug.c:(.text+0x1a): undefined reference to `debug'
collect2: error: ld returned 1 exit status
```

Whoops. What happened here? Well when we used `-DDEBUG` the debug macro
was defined, so it worked as expected. When we don’t compile with
`-DDEBUG` the `#define` debug is never declared in our file so it is
never substituted in our program. Since we used `debug` in the middle of
our code the preprocessor and compiler have no idea what `debug` symbol
is so it fails. Luckily this is easy to fix. We simply have to add
another case to our preprocessor if, else statement to handle this
case.

debug.h:

```c
#ifndef DEBUG_H
#define DEBUG_H
#include<stdlib.h>
#include<stdio.h>

#if DEBUG
	#define debug(msg) printf("DEBUG: %s", msg)
#else
	#define debug(msg)
#endif

#endif
```

main.c:

```c
#include"debug.h"

int main(int argc, char *argv[]) {
	debug("Debug flag was defined\n");
	printf("Hello, World!\n");
	return EXIT_SUCCESS;
}
```

Here we tell the preprocessor to replace any occurrences of `debug(msg)`
with nothing, so now when we don’t compile with `-DDEBUG`. The
preprocessor simply replaces `debug("Debug flag was defined\n")` with
an empty space. Let's compile again.

```
$ make clean all
$ bin/hw1
Hello, World!
```

Cool. Now we can embed debug macros all over our program that look
like normal functions. There’s still a few more cool tricks we can do
to make this better.The preprocessor has a few special macros defined
called ``__LINE__``, ``__FILE__``, and ``__FUNCTION__``. These macros will be
replaced by the preprocessor to evaluate to the *line number* where the
macro is called, the *file name* that the macro is called in, and the
*function name* that the macro is called in. Let's play with this a bit.

debug.h:

```c
#ifndef DEBUG_H
#define DEBUG_H
#include<stdlib.h>
#include<stdio.h>

#ifdef DEBUG
	#define debug(msg) printf("DEBUG: %s:%s:%d %s", __FILE__, __FUNCTION__, __LINE__,msg)
#else
	#define debug(msg)
#endif

#endif
```

main.c:

```c
#include"debug.h"
int main(int argc, char *argv[]) {
	debug("Debug flag was defined\n");
	printf("Hello, World!\n");
	return EXIT_SUCCESS;
}
```

Let's compile this program and run.

```
$ make clean debug
$ bin/hw1
DEBUG: debug.c:main:11 Debug flag was defined
Hello, World!
```

As you can see all the `__FILE__`, `__FUNCTION__`, and `__LINE__` were
replaced with the corresponding values for when debug was called in the
program. Pretty cool, but we can still do even better! Normally when
we want to print something we use `printf()` and use the format
specifiers and variable arguments to print useful information. With our
current setup though we can’t do that. Fortunately for us the
preprocessor offers up a `__VA_ARGS__` macro which we can use to
accomplish this.

> I want to point out that the syntax for this gets a bit crazy and hard
to understand (complex preprocessor stuff is a bit of a black
art). I’ll try my best to describe it but you may need to do some more
googling if the below explanation is not sufficient.

```c
#ifndef DEBUG_H
#define DEBUG_H
#include <stdlib.h>
#include <stdio.h>

#ifdef DEBUG
	#define debug(fmt, ...) printf("DEBUG: %s:%s:%d " fmt, __FILE__, __FUNCTION__,__LINE__, ##__VA_ARGS__)
#else
	#define debug(fmt, ...)
#endif

#endif

#include"debug.h"

int main(int argc, char *argv[]) {
	debug("Program has %d args\n", argc);
	printf("Hello, World!\n");
	return EXIT_SUCCESS;
}
```

First let's compile and run the program and see the results.

```
$ make clean debug
$ bin/hw1
DEBUG: debug.c:main:11 Program has 1 args
Hello, World!
$ make clean all
$ bin/hw1
Hello, World!
```

The macro works as expected, but let's try to explain it a bit.

First we changed the definition of the macro to be `#define debug(fmt, ...)`.
The first argument `fmt` is the format string that we normally
define for printf and `...` is the way to declare a macro that accepts a
variable number of arguments.

Next we have `"DEBUG: %s:%s:%d " fmt`. The C compiler can **concatenate
string literals** that are next to each other. So if `fmt` was the string
`"crazy %d concatenation"` then this statements evaluates to
`"DEBUG:%s:%s:%d crazy %d concatenation"`. Then we have our predefined
preprocessor macros that are used for the string `"DEBUG: %s:%s:%d "`,
and then we reach this next confusing statement: ,
`##__VA_ARGS__`. The macro `__VA_ARGS__` will expand into the variable
arguments provided to the debug statement, but then we have this crazy
`, ##`. This is a hack for allowing no arguments to be passed to the
debug macro, Ex. `debug("I have no varargs")`. If we didn’t do this, the
previous debug statement would throw an warning/error during
the compilation process as it would expect a `__VA_ARGS__` value.

This is one of the many interesting things we can use the C
preprocessor for. Lastly preprocessor macros are in-text replacement
before compilation, this can mean dangerous things when we are
careless about how we use them. For example it is customary to never
put a ; inside a macro definition since most programers would put a
semicolon after the macro as they would most statements. Some
programmers like to wrap the code in macros with a `do{ /*some code
here */ } while(false)` loop. They do this because if your macro is made
up of multiple statements, it will force you to add ; to all the
statements in the do while loop. Then you still have to terminate
this macro with a ; when you use it which makes it seem like a normal
function in your C code.

Our final product will look like this:

```c
#ifndef DEBUG_H
#define DEBUG_H
#include <stdlib.h>
#include <stdio.h>

#ifdef DEBUG
	#define debug(fmt, ...) do{printf("DEBUG: %s:%s:%d " fmt, __FILE__, __FUNCTION__,__LINE__, ##__VA_ARGS__)}while(0)
#else
	#define debug(fmt, ...)
#endif

#endif
```
