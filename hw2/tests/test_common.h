#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define TEST_TIMEOUT 15

#define PROGNAME "bin/par"
#define TEST_REF_DIR "tests/rsrc"
#define TEST_OUTPUT_DIR "test_output"

extern int errors, warnings;

extern char program_options[500];
extern char test_output_subdir[100];
extern char test_log_outfile[100];

int setup_test(char *name);
int run_using_system(char *name, char *limits, char *pre_cmd, char *valgrind_cmd);
void assert_normal_exit(int status);
void assert_error_exit(int status);
void assert_expected_status(int expected, int status);
void assert_expected_status(int expected, int status);
void assert_signaled(int sig, int status);
void assert_outfile_matches(char *name, char *filter);
void assert_errfile_matches(char *name, char *filter);
void assert_no_valgrind_errors(int status);
