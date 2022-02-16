#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

char parse_arg(char *arg);
int str2int(char *arg);

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

int validargs(int argc, char **argv)
{
    global_options = 0;
    int isValid = 0;
    char **ptr = argv;
    switch (parse_arg(*(++ptr)))
    {
    case 'h':
        global_options = HELP_OPTION;
        isValid = 1;
        break;
    case 'v':
        if (argc == 2)
        {
            global_options = VALIDATE_OPTION;
            isValid = 1;
        }
        break;
    case 'c':
        if (argc == 2)
        {
            global_options = CANONICALIZE_OPTION;
            isValid = 1;
        }
        else
        {
            if (parse_arg(*(++ptr)) == 'p')
            {
                int indented_value;
                if (argc == 3) {
                    indented_value = 4;
                }
                else {
                    indented_value = str2int(*(++ptr));
                }
                if (indented_value == -1) {
                    break;
                }
                global_options = CANONICALIZE_OPTION + PRETTY_PRINT_OPTION + indented_value;
                isValid = 1;
                break;
            }
        }
        break;
    default:
        break;
    }
    if (isValid)
    {
        return 0;
    }
    return -1;
}

char parse_arg(char *arg)
{
    if (*arg == '-')
    {
        char arg1 = *(++arg), arg2 = *(++arg);
        if (arg2 == '\0')
        {
            return arg1;
        }
    }
    return '\0';
}

int str2int(char *arg)
{
    int ans = 0;
    while (*arg != '\0')
    {
        if (*arg <= '9' && *arg >= '0')
        {
            ans = ans * 10 + (*arg - '0');
            arg++;
        }
        else
        {
            ans = -1;
            break;
        }
    }
    if (ans < 256 && ans >= 0) {
        return ans;
    }
    else {
        return -1;
    }
}