#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "store.h"

/*
 * This is the "data store" module for Mush.
 * It maintains a mapping from variable names to values.
 * The values of variables are stored as strings.
 * However, the module provides functions for setting and retrieving
 * the value of a variable as an integer.  Setting a variable to
 * an integer value causes the value of the variable to be set to
 * a string representation of that integer.  Retrieving the value of
 * a variable as an integer is possible if the current value of the
 * variable is the string representation of an integer.
 */

/**
 * @brief  Get the current value of a variable as a string.
 * @details  This function retrieves the current value of a variable
 * as a string.  If the variable has no value, then NULL is returned.
 * Any string returned remains "owned" by the data store module;
 * the caller should not attempt to free the string or to use it
 * after any subsequent call that would modify the value of the variable
 * whose value was retrieved.  If the caller needs to use the string for
 * an indefinite period, a copy should be made immediately.
 *
 * @param  var  The variable whose value is to be retrieved.
 * @return  A string that is the current value of the variable, if any,
 * otherwise NULL.
 */
char *store_get_string(char *var) {
    store_data *data = store_get_data(var);
    if (data)
        return data->value;
    return NULL;
}

/**
 * @brief  Get the current value of a variable as an integer.
 * @details  This retrieves the current value of a variable and
 * attempts to interpret it as an integer.  If this is possible,
 * then the integer value is stored at the pointer provided by
 * the caller.
 *
 * @param  var  The variable whose value is to be retrieved.
 * @param  valp  Pointer at which the returned value is to be stored.
 * @return  If the specified variable has no value or the value
 * cannot be interpreted as an integer, then -1 is returned,
 * otherwise 0 is returned.
 */
int store_get_int(char *var, long *valp) {
    store_data *data = store_get_data(var);
    if (data) {
        // is NULL
        char *ptr = data->value;
        if (!ptr)
            return -1;

        // find whether negative or not
        int sign = 1;
        if (*ptr == '-')
            sign = -1, ptr ++;

        long ans = 0;
        while (*ptr)
        {
            // cannot be interpreted as an integer
            if(*ptr > '9' || *ptr < '0')
                return -1;
            ans = ans * 10 +(*(ptr++) - '0');
        }
        *valp = ans * sign;
    }
    return -1;
}

/**
 * @brief  Set the value of a variable as a string.
 * @details  This function sets the current value of a specified
 * variable to be a specified string.  If the variable already
 * has a value, then that value is replaced.  If the specified
 * value is NULL, then any existing value of the variable is removed
 * and the variable becomes un-set.  Ownership of the variable and
 * the value strings is not transferred to the data store module as
 * a result of this call; the data store module makes such copies of
 * these strings as it may require.
 *
 * @param  var  The variable whose value is to be set.
 * @param  val  The value to set, or NULL if the variable is to become
 * un-set.
 */
int store_set_string(char *var, char *val) {
    store_data* data = store_get_data(var);
    // if want to unset a value that does not exist
    if (!data && !val)
        return -1;

    // unset is val is NULL
    if (!val)
    {
        store_remove_data(data);
        return 0;
    }
    
    // set the data to val if data exists
    if (data)
    {
        char *tmp = data->value;
        data->value = store_strcpy(val);
        free(tmp);
        return 0;
    }
    
    // create a new data with the value and insert it
    data = malloc(sizeof(store_data));
    data->name = store_strcpy(var);
    data->value = store_strcpy(val);
    store_add_data(data);
    return 0;
}

/**
 * @brief  Set the value of a variable as an integer.
 * @details  This function sets the current value of a specified
 * variable to be a specified integer.  If the variable already
 * has a value, then that value is replaced.  Ownership of the variable
 * string is not transferred to the data store module as a result of
 * this call; the data store module makes such copies of this string
 * as it may require.
 *
 * @param  var  The variable whose value is to be set.
 * @param  val  The value to set.
 */
int store_set_int(char *var, long val) {
    // if val is 0, set var to 0
    if (val == 0)
        return store_set_string(var, "0");
    
    char *str, *tmp, *ptr;
    tmp = malloc(sizeof(char));
    int is_negative = 0, size = 0;

    // check whether val is negative
    if (val < 0)
        is_negative = 1;
    
    // get a string with reversed direction
    while (val)
    {
        int remainder = val % 10;
        if (is_negative)
            remainder *= -1;
        tmp[size] = ('0' + remainder);
        tmp = realloc(tmp, (++size + 1) * sizeof(char));
        val /= 10;
    }

    // alloc memory to str
    if (is_negative) {
        str = malloc((size+2)*sizeof(char));
        ptr = str + 1;
        str[0] = '-';
    }
    else {
        str = malloc((size+1)*sizeof(char));
        ptr = str;
    }

    // reverse the string back
    for (size_t i = 0; i < size; i++)
        ptr[i] = tmp[size - i - 1];
    
    ptr[size] = '\0';

    // put it in data
    int return_val = store_set_string(var, str);

    free(str);
    free(tmp);
    
    return return_val;
}

/**
 * @brief  Print the current contents of the data store.
 * @details  This function prints the current contents of the data store
 * to the specified output stream.  The format is not specified; this
 * function is intended to be used for debugging purposes.
 *
 * @param f  The stream to which the store contents are to be printed.
 */
void store_show(FILE *f) {
    store_data* ptr = head->next;

    fprintf(f, "{");
    while (ptr != head)
    {
        fprintf(f, "%s=%s", ptr->name, ptr->value);
        ptr = ptr->next;
        if ((ptr != head))
            fprintf(f, ", ");
        
    }
    fprintf(f, "}\n");
}

void store_init() {
    head = malloc(sizeof(store_data));
    head->name = NULL;
    head->value = NULL;
    head->next = head;
    head->prev = head;
}

store_data* store_get_data(char* var) {
    store_data* ptr = head->next;

    while (ptr != head)
    {
        if (strcmp(ptr->name, var) == 0)
            return ptr;

        ptr = ptr->next;
    }

    return NULL;
}

void store_remove_data(store_data* data) {
    data->prev->next = data->next;
    data->next->prev = data->prev;
    if (data->name)
        free(data->name);
    if (data->value)
        free(data->value);
    free(data);
}

void store_add_data(store_data* data) {
    data->next = head->next;
    data->prev = head;
    data->next->prev = data;
    data->prev->next = data;
}

void store_fini() {
    while (head->next != head)
        store_remove_data(head->next);

    free(head);
    head = NULL;
}

char* store_strcpy(char *str) {
    size_t size = 0;
    char *result = malloc(sizeof(char));
    while (*str)
    {
        result[size] = *(str++);
        result = realloc(result, (++size + 1) * sizeof(char));
    }
    result[size] = '\0';
    return result;
}