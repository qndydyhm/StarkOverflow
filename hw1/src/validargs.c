#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

int parse_arg(char *arg);
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
    case 8:
        global_options = HELP_OPTION;
        isValid = 1;
        break;
    case 4:
        if (argc == 2)
        {
            global_options = VALIDATE_OPTION;
            isValid = 1;
        }
        break;
    case 2:
        if (argc == 2)
        {
            global_options = CANONICALIZE_OPTION;
            isValid = 1;
        }
        else
        {
            if (parse_arg(*(++ptr)) == 1)
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

int parse_arg(char *arg)
{
    int ans = -1;
    if (*arg == '-')
    {
        char arg1 = *(++arg), arg2 = *(++arg);
        if (arg2 == '\0')
        {
            if (arg1 == 'h')
            {
                ans = 8;
            }
            else if (arg1 == 'v')
            {
                ans = 4;
            }
            else if (arg1 == 'c')
            {
                ans = 2;
            }
            else if (arg1 == 'p')
            {
                ans = 1;
            }
        }
    }
    return ans;
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