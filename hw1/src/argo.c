#include <stdlib.h>
#include <stdio.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

ARGO_VALUE *argo_get_next_value();
char argo_get_next_char(FILE *f);
int argo_get_prev_char(FILE *f);
int print_error_helper();
int argo_read_string(ARGO_STRING *s, FILE *f);
int argo_read_number(ARGO_NUMBER *n, FILE *f);
int argo_read_true(ARGO_BASIC *b, FILE *f);
int argo_read_false(ARGO_BASIC *b, FILE *f);
int argo_read_null(ARGO_BASIC *b, FILE *f);
int argo_read_object(ARGO_OBJECT *o, FILE *f);
int argo_read_object_helper(ARGO_VALUE *v, FILE *f);
int argo_read_array(ARGO_ARRAY *a, FILE *f);
int argo_write_basic(ARGO_BASIC *b, FILE *f);
int argo_write_object(ARGO_VALUE *o, FILE *f);
int argo_write_array(ARGO_VALUE *a, FILE *f);
int argo_write_number_helper(ARGO_STRING *s, FILE *f);
int print_indent(FILE *f);

char latest_char;
int argo_chars_read_prev;

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
ARGO_VALUE *argo_read_value(FILE *f)
{
    ARGO_VALUE *v = argo_get_next_value();
    int i = argo_get_next_char(f);
    while (i != EOF)
    {
        if (argo_is_whitespace(i))
        {
            i = argo_get_next_char(f);
            continue;
        }
        else if (argo_is_control(i))
        {
            print_error_helper(i);
            return NULL;
        }
        else if (argo_is_digit(i) || i == '-')
        {
            argo_get_prev_char(f);
            if (argo_read_number(&v->content.number, f))
            {
                return NULL;
            }
            else
            {
                v->type = ARGO_NUMBER_TYPE;
                return v;
            }
        }
        else if (i == '{')
        {
            if (argo_read_object(&v->content.object, f))
            {
                return NULL;
            }
            else
            {
                v->type = ARGO_OBJECT_TYPE;
                return v;
            };
        }
        else if (i == '[')
        {
            if (argo_read_array(&v->content.array, f))
            {
                return NULL;
            }
            else
            {
                v->type = ARGO_ARRAY_TYPE;
                return v;
            };
        }
        else if (i == '"')
        {
            if (argo_read_string(&v->content.string, f))
            {
                return NULL;
            }
            else
            {
                v->type = ARGO_STRING_TYPE;
                return v;
            };
        }
        else if (i == 't')
        {
            if (argo_read_true(&v->content.basic, f))
            {
                return NULL;
            }
            else
            {
                v->type = ARGO_BASIC_TYPE;
                return v;
            };
        }
        else if (i == 'f')
        {
            if (argo_read_false(&v->content.basic, f))
            {
                return NULL;
            }
            else
            {
                v->type = ARGO_BASIC_TYPE;
                return v;
            }
        }
        else if (i == 'n')
        {
            if (argo_read_null(&v->content.basic, f))
            {
                return NULL;
            }
            else
            {
                v->type = ARGO_BASIC_TYPE;
                return v;
            }
        }
        else
        {
            print_error_helper();
            return NULL;
        }
        argo_get_next_char(f);
    }

    return NULL;
}

ARGO_VALUE *argo_get_next_value()
{
    ARGO_VALUE *v = argo_value_storage;
    if (argo_next_value >= NUM_ARGO_VALUES)
    {
        fprintf(stderr, "Nums of values exceed the maxium \"%d\" values.", NUM_ARGO_VALUES);
        abort();
    }
    v += argo_next_value++;
    return v;
}
int print_error_helper()
{
    fprintf(stderr, "Unexpected charactor %c at [%d, %d].", latest_char, argo_lines_read, argo_chars_read);
    return -1;
}

char argo_get_next_char(FILE *f)
{
    char c = fgetc(f);
    latest_char = c;
    argo_chars_read_prev = argo_chars_read;
    argo_chars_read++;
    if (c == '\n')
    {
        argo_lines_read++;
        argo_chars_read = 0;
    }
    return c;
}

int argo_get_prev_char(FILE *f)
{
    char c = ungetc(latest_char, f);
    argo_chars_read--;
    if (c == '\n')
    {
        argo_lines_read--;
        argo_chars_read = argo_chars_read_prev;
    }
    return 0;
}

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
int argo_read_string(ARGO_STRING *s, FILE *f)
{
    ARGO_CHAR c = argo_get_next_char(f);
    while (c != EOF)
    {
        if (argo_is_control(c))
        {
            return print_error_helper();
        }
        if (c == '"')
        {
            break;
        }
        if (c == '\\')
        {
            c = argo_get_next_char(f);
            int tmp = 0, value = 0;
            switch (c)
            {
            case '/':
                c = '/';
                break;
            case '"':
                c = '"';
                break;
            case '\\':
                c = '\\';
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case 'u':

                for (size_t i = 0; i < 4; i++)
                {
                    c = argo_get_next_char(f);
                    if (!argo_is_hex(c))
                    {
                        return print_error_helper();
                    }
                    if (argo_is_digit(c))
                    {
                        tmp = c - '0';
                    }
                    else if (c <= 'f' && c >= 'a')
                    {
                        tmp = c - 'a' + 10;
                    }
                    else
                    {
                        tmp = c - 'A' + 10;
                    }
                    value = value * 16 + tmp;
                }
                c = value;
                break;
            default:
                break;
            }
        }
        argo_append_char(s, c);
        c = argo_get_next_char(f);
    }
    return 0;
}

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
int argo_read_number(ARGO_NUMBER *n, FILE *f)
{
    int has_exp = 0, num_started = 0, exp_started = 0, has_separator = 0, valid_int = 1, valid_float = 1, valid_string = 1, num_is_positive = 1, digits_after_separator = 0;
    long int_value = 0;
    double float_value = 0;
    ARGO_CHAR *string_value = argo_digits;
    size_t length = 0;
    char c = argo_get_next_char(f);
    while (1)
    {
        if (!(argo_is_digit(c) || c == '+' || c == '-' || c == 'e' || c == 'E' || c == '.'))
        {
            break;
        }
        if (length < 9)
        {
            ARGO_CHAR *ptr = string_value;
            ptr += length;
            if (c == 'E')
            {
                c = 'e';
            }
            
            *ptr = c;
            length++;
        }
        else
        {
            valid_string = 0;
        }
        if (argo_is_digit(c))
        {
            if (valid_int)
            {
                int_value = int_value * 10 + (c - '0');
                if (num_started && int_value == 0 && c == '0')
                {
                    return print_error_helper();
                }
            }
            if (valid_float)
            {
                if (has_separator)
                {
                    digits_after_separator++;
                    float_value += (c - '0') / (10 ^ digits_after_separator);
                }
                else
                {
                    float_value = float_value * 10 + (c - '0');
                }
            }
        }
        switch (c)
        {
        case '-':
            if (!num_started)
            {
                num_is_positive = -1;
                c = argo_get_next_char(f);
                continue;
            }
            else if (has_exp && !exp_started)
            {
            }
            else
            {
                return print_error_helper();
            }
            break;
        case '+':
            if (has_exp && !exp_started)
            {
            }
            else
            {
                return print_error_helper();
            }
            break;
        case 'e':
        case 'E':
            if (has_exp)
            {
                return print_error_helper();
            }
            valid_float = 0;
            valid_int = 0;
            has_exp = 1;
            c = argo_get_next_char(f);
            continue;
            break;
        case '.':
            if (has_separator)
            {
                return print_error_helper();
            }
            valid_int = 0;
            has_separator = 1;
            break;
        default:
            break;
        }
        num_started = 1;
        if (has_exp)
        {
            exp_started = 1;
        }
        c = argo_get_next_char(f);
    }
    if (!valid_int && !valid_float && !valid_string)
    {
        fprintf(stderr, "num at [%d, %d] is in exponential format and too long(larger than 10 digits", argo_lines_read, argo_chars_read);
    }
    int_value *= num_is_positive;
    float_value *= num_is_positive;
    n->int_value = int_value;
    n->float_value = float_value;
    for (size_t i = 0; i < length; i++)
    {
        ARGO_CHAR *tmp = string_value;
        tmp += (i);
        argo_append_char(&n->string_value, *tmp);
    }

    n->valid_int = valid_int;
    n->valid_float = valid_float;
    n->valid_string = valid_string;
    argo_get_prev_char(f);
    return 0;
}

int argo_read_true(ARGO_BASIC *b, FILE *f)
{
    char c = argo_get_next_char(f);
    if (c == 'r')
    {
        c = argo_get_next_char(f);
        if (c == 'u')
        {
            c = argo_get_next_char(f);
            if (c == 'e')
            {
                *b = ARGO_TRUE;
                return 0;
            }
        }
    }
    return print_error_helper();
}

int argo_read_false(ARGO_BASIC *b, FILE *f)
{
    char c = argo_get_next_char(f);
    if (c == 'a')
    {
        c = argo_get_next_char(f);
        if (c == 'l')
        {
            c = argo_get_next_char(f);
            if (c == 's')
            {
                c = argo_get_next_char(f);
                if (c == 'e')
                {
                    *b = ARGO_FALSE;
                    return 0;
                }
            }
        }
    }
    return print_error_helper();
}

int argo_read_null(ARGO_BASIC *b, FILE *f)
{
    char c = argo_get_next_char(f);
    if (c == 'u')
    {
        c = argo_get_next_char(f);
        if (c == 'l')
        {
            c = argo_get_next_char(f);
            if (c == 'l')
            {
                *b = ARGO_NULL;
                return 0;
            }
        }
    }
    return print_error_helper();
}

int argo_read_object(ARGO_OBJECT *o, FILE *f)
{
    char c = argo_get_next_char(f);
    o->member_list = argo_get_next_value();
    ARGO_VALUE *starter = o->member_list;
    starter->type = ARGO_NO_TYPE;
    starter->next = starter;
    starter->prev = starter;
    ARGO_VALUE *prev = starter;
    ARGO_VALUE *next = argo_get_next_value();

    while (1)
    {
        while (argo_is_whitespace(c))
        {
            c = argo_get_next_char(f);
        }

        if (c == '}')
        {
            break;
        }
        else if (c == '\"')
        {
            if (argo_read_string(&next->name, f))
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
        c = argo_get_next_char(f);

        while (argo_is_whitespace(c))
        {
            c = argo_get_next_char(f);
        }

        if (c != ':')
        {
            return -1;
        }
        if (argo_read_object_helper(next, f))
        {
            return -1;
        }
        next->prev = prev;
        prev->next = next;
        next->next = starter;
        starter->prev = next;
        prev = next;
        next = argo_get_next_value();
        c = argo_get_next_char(f);
        while (argo_is_whitespace(c))
        {
            c = argo_get_next_char(f);
        }
        if (c == '}')
        {
            break;
        }
        else if (c == ',')
        {
            c = argo_get_next_char(f);
            continue;
        }
    }
    return 0;
}

int argo_read_object_helper(ARGO_VALUE *v, FILE *f)
{
    ARGO_VALUE *tmp = argo_read_value(f);
    if (tmp == NULL)
    {
        return -1;
    }
    v->type = tmp->type;
    v->content = tmp->content;
    return 0;
}

int argo_read_array(ARGO_ARRAY *a, FILE *f)
{
    char c = argo_get_next_char(f);
    a->element_list = argo_get_next_value();
    ARGO_VALUE *starter = a->element_list;
    starter->type = ARGO_NO_TYPE;
    starter->next = starter;
    starter->prev = starter;
    ARGO_VALUE *prev = starter;
    ARGO_VALUE *next = argo_get_next_value();

    while (1)
    {
        while (argo_is_whitespace(c))
        {
            c = argo_get_next_char(f);
        }

        if (c == ']')
        {
            break;
        }
        else {
            argo_get_prev_char(f);
        }

        if (argo_read_object_helper(next, f))
        {
            return -1;
        }
        next->prev = prev;
        prev->next = next;
        next->next = starter;
        starter->prev = next;
        prev = next;
        next = argo_get_next_value();
        c = argo_get_next_char(f);
        while (argo_is_whitespace(c))
        {
            c = argo_get_next_char(f);
        }
        if (c == ']')
        {
            break;
        }
        else if (c == ',')
        {
            c = argo_get_next_char(f);
            continue;
        }
    }
    return 0;
}

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
        return argo_write_basic(&v->content.basic, f);
        break;
    case ARGO_NUMBER_TYPE:
        return argo_write_number(&v->content.number, f);
        break;
    case ARGO_STRING_TYPE:
        return argo_write_string(&v->content.string, f);
        break;
    case ARGO_OBJECT_TYPE:
        return argo_write_object(v->content.object.member_list, f);
        break;
    case ARGO_ARRAY_TYPE:
        return argo_write_array(v->content.array.element_list, f);
        break;
    default:
        return 1;
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
                if (*c <= 0xff)
                {
                    fprintf(f, "%c", *c);
                }
                else if (*c <= 0xfff)
                {
                    fprintf(f, "\\u0%x", *c);
                }
                else {
                    fprintf(f, "\\u%x", *c);
                }
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
        if (global_options & PRETTY_PRINT_OPTION)
        {
            fprintf(f, " ");
        }

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
    if (indent_level == 0)
    {
        fprintf(f, "\n");
    }
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
    if (indent_level == 0)
    {
        fprintf(f, "\n");
    }
    
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