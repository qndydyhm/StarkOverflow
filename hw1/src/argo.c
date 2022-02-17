#include <stdlib.h>
#include <stdio.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

int argo_write_basic(ARGO_BASIC *b, FILE *f);
int argo_write_object(ARGO_VALUE *o, FILE *f);
int argo_write_array(ARGO_VALUE *a, FILE *f);
int argo_write_number_helper(ARGO_STRING *s, FILE *f);
int print_indent(FILE *f);
/**
 * @brief  Read JSON input from a specified input stream, parse it,
 * and return a data structure representing the corresponding value.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON value,
 * according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  See the assignment handout for
 * information on the JSON syntax standard and how parsing can be
 * accomplished.  As discussed in the assignment handout, the returned
 * pointer must be to one of the elements of the argo_value_storage
 * array that is defined in the const.h header file.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  A valid pointer if the operation is completely successful,
 * NULL if there is any error.
 */
// ARGO_VALUE *argo_read_value(FILE *f) {
//     // TO BE IMPLEMENTED.

//     return NULL;
// }

// ARGO_VALUE *argo_read_value_helper(FILE *f, ARGO_VALUE *this) {
//     int i = fgetc(f);
//     int started = 0;
//     while (i != EOF) {
//         switch (i)
//         {
//         case '\b':
//         case '\f':
//         case '\r':
//         case '\t':
//             break;
//         case '\n':
//             argo_lines_read++;
//             argo_chars_read = 0;
//         default:
//             break;
//         }
//         i = fgetc(f);
//     }
//     return this;
// }

/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON string literal, and return a data structure
 * representing the corresponding string.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON string
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
// int argo_read_string(ARGO_STRING *s, FILE *f) {
// TO BE IMPLEMENTED.
// abort();
// }

/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON number, and return a data structure representing
 * the corresponding number.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON numeric
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  The returned value must contain
 * (1) a string consisting of the actual sequence of characters read from
 * the input stream; (2) a floating point representation of the corresponding
 * value; and (3) an integer representation of the corresponding value,
 * in case the input literal did not contain any fraction or exponent parts.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
// int argo_read_number(ARGO_NUMBER *n, FILE *f) {
// TO BE IMPLEMENTED.
// abort();
// }

/**
 * @brief  Write canonical JSON representing a specified value to
 * a specified output stream.
 * @details  Write canonical JSON representing a specified value
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.
 *
 * @param v  Data structure representing a value.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_value(ARGO_VALUE *v, FILE *f)
{
    switch (v->type)
    {
    case ARGO_BASIC_TYPE:
        argo_write_basic(&v->content.basic, f);
        break;
    case ARGO_NUMBER_TYPE:
        argo_write_number(&v->content.number, f);
        break;
    case ARGO_STRING_TYPE:
        argo_write_string(&v->content.string, f);
        break;
    case ARGO_OBJECT_TYPE:
        argo_write_object(v->content.object.member_list, f);
        break;
    case ARGO_ARRAY_TYPE:
        argo_write_array(v->content.array.element_list, f);
        break;
    default:
        abort();
        break;
    }
    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified string
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified string
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument string may contain any sequence of
 * Unicode code points and the output is a JSON string literal,
 * represented using only 8-bit bytes.  Therefore, any Unicode code
 * with a value greater than or equal to U+00FF cannot appear directly
 * in the output and must be represented by an escape sequence.
 * There are other requirements on the use of escape sequences;
 * see the assignment handout for details.
 *
 * @param v  Data structure representing a string (a sequence of
 * Unicode code points).
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_basic(ARGO_BASIC *b, FILE *f)
{
    switch (*b)
    {
    case ARGO_NULL:
        fprintf(f, ARGO_NULL_TOKEN);
        break;
    case ARGO_TRUE:
        fprintf(f, ARGO_TRUE_TOKEN);
        break;
    case ARGO_FALSE:
        fprintf(f, ARGO_FALSE_TOKEN);
        break;
    default:
        return -1;
        break;
    }
    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified number
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified number
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument number may contain representations
 * of the number as any or all of: string conforming to the
 * specification for a JSON number (but not necessarily canonical),
 * integer value, or floating point value.  This function should
 * be able to work properly regardless of which subset of these
 * representations is present.
 *
 * @param v  Data structure representing a number.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */

int argo_write_number(ARGO_NUMBER *n, FILE *f)
{
    if (n->valid_int)
    {
        fprintf(f, "%ld", n->int_value);
    }
    else if (n->valid_float && n->float_value < 1 && n->float_value >= 0.1)
    {
        fprintf(f, "%f", n->float_value);
    }
    else if (n->valid_string)
    {
        return argo_write_number_helper(&n->string_value, f);
    }
    else
    {
        return -1;
    }
    return 0;
}

int argo_write_number_helper(ARGO_STRING *s, FILE *f)
{
    int *c = s->content;
    int num = 0, exp = 0, exp_tmp = 0, is_before_separator = 1, num_is_plus = 1, started = 0, exp_is_plus = 1, is_num = 1, integer_is_zero = 0, fractional_is_zero = 1;
    for (size_t index = 0; index < s->length; index++, c++)
    {
        if (is_num)
        {
            if (argo_is_digit(*c))
            {
                num = num * 10 + (*c - '0');
                if (is_before_separator && num)
                {
                    exp += 1;
                }
                if (integer_is_zero && fractional_is_zero)
                {
                    if (*c == '0')
                    {
                        exp -= 1;
                    }
                    else
                    {
                        fractional_is_zero = 0;
                    }
                }
            }
            else if (argo_is_exponent(*c))
            {
                started = 0, is_num = 0;
                continue;
            }
            else if (*c == '-' && !started)
            {
                num_is_plus = -1;
            }
            else if (*c == '+' && !started)
            {
            }
            else if (*c == '.' && is_before_separator)
            {
                is_before_separator = 0;
                if (!num)
                {
                    integer_is_zero = 1;
                }
            }
            else
            {
                return -1;
            }
            started = 1;
        }
        else
        {
            if (argo_is_digit(*c))
            {
                exp_tmp = exp_tmp * 10 + (*c - '0');
            }
            else if (*c == '-' && !started)
            {
                exp_is_plus = -1;
            }
            else if (*c == '+' && !started)
            {
            }
            else
            {
                return -1;
            }
            started = 1;
        }
    }
    exp = exp_is_plus * exp_tmp + exp;
    if (num_is_plus == -1)
    {
        fprintf(f, "-");
    }
    while (num % 10 == 0 && num != 0)
    {
        num /= 10;
    }

    fprintf(f, "0.%d", num);
    if (exp && num != 0)
    {
        fprintf(f, "e%d", exp);
    }
    return 0;
}
int argo_write_string(ARGO_STRING *s, FILE *f)
{
    fprintf(f, "\"");
    int *c = s->content;
    for (size_t index = 0; index < s->length; index++, c++)
    {
        switch (*c)
        {
        case ARGO_BS:
            fprintf(f, "\\b");
            break;
        case ARGO_FF:
            fprintf(f, "\\f");
            break;
        case ARGO_LF:
            fprintf(f, "\\n");
            break;
        case ARGO_CR:
            fprintf(f, "\\r");
            break;
        case ARGO_HT:
            fprintf(f, "\\t");
            break;
        case ARGO_BSLASH:
            fprintf(f, "\\\\");
            break;
        case ARGO_QUOTE:
            fprintf(f, "\\\"");
            break;
        default:
            if (argo_is_control(*c))
            {
                int tmp = 0xf;
                char lsb = *c & 0xff;
                if (lsb <= 0xf)
                {
                    fprintf(f, "\\u000%x", lsb);
                }
                else
                {
                    fprintf(f, "\\u00%x", lsb);
                }
            }
            else
            {
                fprintf(f, "%c", *c);
            }
            break;
        }
    }
    fprintf(f, "\"");
    return 0;
}

int argo_write_object(ARGO_VALUE *o, FILE *f)
{
    fprintf(f, "{");
    indent_level++;
    print_indent(f);
    ARGO_VALUE *ptr = o;
    while (ptr->next->type != ARGO_NO_TYPE)
    {
        argo_write_string(&ptr->next->name, f);
        fprintf(f, ":");
        argo_write_value(ptr->next, f);
        ptr = ptr->next;
        if (ptr->next->type != ARGO_NO_TYPE)
        {
            fprintf(f, ",");
            print_indent(f);
        }
    }
    indent_level--;
    print_indent(f);
    fprintf(f, "}");
    return 0;
}

int argo_write_array(ARGO_VALUE *a, FILE *f)
{
    fprintf(f, "[");
    indent_level++;
    print_indent(f);
    ARGO_VALUE *ptr = a;
    while (ptr->next->type != ARGO_NO_TYPE)
    {
        argo_write_value(ptr->next, f);
        ptr = ptr->next;
        if (ptr->next->type != ARGO_NO_TYPE)
        {
            fprintf(f, ",");
            print_indent(f);
        }
    }
    indent_level--;
    print_indent(f);
    fprintf(f, "]");
    return 0;
}

int print_indent(FILE *f)
{
    if (global_options & PRETTY_PRINT_OPTION)
    {
        fprintf(f, "\n");
        for (size_t i = 0; i < indent_level; i++)
        {
            for (size_t j = 0; j < (global_options & 0x0fffffff); j++)
            {
                fprintf(f, " ");
            }
        }
    }
    return 0;
}