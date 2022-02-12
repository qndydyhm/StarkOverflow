#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

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
    char **ptr = argv;
    ptr++;
    for (int i = 1; i < argc; i++, ptr++)
    {
        if (**ptr == '-')
        {
            char *arg = *ptr;
            arg++;
            switch (*arg)
            {
            case 'h':
                if (i == 1 && *(++arg) == '\0')
                {
                    global_options = HELP_OPTION;
                    return 0;
                }
                else
                {
                    return -1;
                }
                break;
            case 'v':
                if (i == 1 && *(++arg) == '\0' && argc == 2)
                {
                    global_options = VALIDATE_OPTION;
                    return 0;
                }
                else
                {
                    return -1;
                }
                break;
            case 'c':
                if (i == 1 && *(++arg) == '\0')
                {
                    if (argc == 2)
                    {
                        global_options = CANONICALIZE_OPTION;
                        return 0;
                    }
                    else
                    {
                        ptr++;
                        if (**ptr != '-')
                        {
                            return -1;
                        }
                        arg = *ptr;
                        if (*(++arg) != 'p')
                        {
                            return -1;
                        }
                        if (*(++arg) != '\0')
                        {
                            return -1;
                        }
                        global_options = PRETTY_PRINT_OPTION;
                        if (argc == 3)
                        {
                            return 0;
                        }
                        else if (argc == 4)
                        {
                            int indent_value = 0;
                            arg = *(++ptr);
                            while (*arg != '\0')
                            {
                                if (*arg <= '9' && *arg >= '0')
                                {
                                    indent_value = indent_value * 10 + (*arg - '0');
                                    arg++;
                                }
                                else
                                {
                                    return 1;
                                }
                            }
                            global_options += indent_value;
                            return 0;
                        }
                        else
                        {
                            return -1;
                        }
                    }
                }
                else
                {
                    return -1;
                }
                break;
            default:
                return -1;
                break;
            }
        }
    }
    return -1;
}