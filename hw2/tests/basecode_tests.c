#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "test_common.h"

#define STANDARD_LIMITS "ulimit -t 10; ulimit -f 2000"

/*
 * Start the program and then trigger EOF on input.
 * The program should exit with EXIT_SUCCESS.
 */
Test(base_suite, EOF_test) {
    char *name = "EOF";
    sprintf(program_options, "%s", "");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
}

/*
 * Run the program with default options on a non-empty input file
 * and check the results.
 */
Test(base_suite, basic_test) {
    char *name = "basic";
    sprintf(program_options, "%s", "");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
}

/*
 * Run the program with default options on an input file with
 * prefixes and suffixes and check the results.
 */
Test(base_suite, prefix_suffix_test) {
    char *name = "prefix_suffix";
    sprintf(program_options, "%s", "w80");
    int err = run_using_system(name, "", "", STANDARD_LIMITS);
    assert_expected_status(EXIT_SUCCESS, err);
    assert_outfile_matches(name, NULL);
}

/*
 * Run the program with default options on a non-empty input file
 * and use valgrind to check for leaks.
 */
Test(base_suite, valgrind_leak_test) {
    char *name = "valgrind_leak";
    sprintf(program_options, "%s", "");
    int err = run_using_system(name, "", "valgrind --leak-check=full --undef-value-errors=no --error-exitcode=37", STANDARD_LIMITS);
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
}

/*
 * Run the program with default options on a non-empty input file
 * and use valgrind to check for uninitialized values.
 */
Test(base_suite, valgrind_uninitialized_test) {
    char *name = "valgrind_uninitialized";
    sprintf(program_options, "%s", "p10 s10");
    int err = run_using_system(name, "", "valgrind --leak-check=no --undef-value-errors=yes --error-exitcode=37", STANDARD_LIMITS);
    assert_no_valgrind_errors(err);
    assert_expected_status(0x1, err);
    assert_outfile_matches(name, NULL);
}

