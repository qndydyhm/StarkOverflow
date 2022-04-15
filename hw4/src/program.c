#include <stdlib.h>
#include <stdio.h>

#include "mush.h"
#include "debug.h"
#include "program.h"

/*
 * This is the "program store" module for Mush.
 * It maintains a set of numbered statements, along with a "program counter"
 * that indicates the current point of execution, which is either before all
 * statements, after all statements, or in between two statements.
 * There should be no fixed limit on the number of statements that the program
 * store can hold.
 */

/**
 * @brief  Output a listing of the current contents of the program store.
 * @details  This function outputs a listing of the current contents of the
 * program store.  Statements are listed in increasing order of their line
 * number.  The current position of the program counter is indicated by
 * a line containing only the string "-->" at the current program counter
 * position.
 *
 * @param out  The stream to which to output the listing.
 * @return  0 if successful, -1 if any error occurred.
 */
int prog_list(FILE *out) {
    prog_data *ptr = prog_head->next;
    while (1)
    {
        if (ptr == prog_counter)
            fprintf(out, "-->\n");
        if (ptr == prog_head)
            break;
        show_stmt(out, ptr->stmt);
        ptr = ptr->next;
    }
    return 0;
}

/**
 * @brief  Insert a new statement into the program store.
 * @details  This function inserts a new statement into the program store.
 * The statement must have a line number.  If the line number is the same as
 * that of an existing statement, that statement is replaced.
 * The program store assumes the responsibility for ultimately freeing any
 * statement that is inserted using this function.
 * Insertion of new statements preserves the value of the program counter:
 * if the position of the program counter was just before a particular statement
 * before insertion of a new statement, it will still be before that statement
 * after insertion, and if the position of the program counter was after all
 * statements before insertion of a new statement, then it will still be after
 * all statements after insertion.
 *
 * @param stmt  The statement to be inserted.
 * @return  0 if successful, -1 if any error occurred.
 */
int prog_insert(STMT *stmt) {
    // if no lineno, return -1
    if (!stmt->lineno)
        return -1;

    // get first stmt after lineno
    prog_data *ptr = prog_get_data(stmt->lineno);

    // if stmt exists
    if (ptr != prog_head && ptr->stmt->lineno == stmt->lineno)
    {
        free_stmt(ptr->stmt);
        ptr->stmt = stmt;
        return 0;
    }
    

    // insert the new one
    prog_data *new = malloc(sizeof(prog_data));
    new->next = ptr;
    new->prev = ptr->prev;
    new->prev->next = new;
    new->next->prev = new;
    new->stmt = stmt;
    return 0;
}

/**
 * @brief  Delete statements from the program store.
 * @details  This function deletes from the program store statements whose
 * line numbers fall in a specified range.  Any deleted statements are freed.
 * Deletion of statements preserves the value of the program counter:
 * if before deletion the program counter pointed to a position just before
 * a statement that was not among those to be deleted, then after deletion the
 * program counter will still point the position just before that same statement.
 * If before deletion the program counter pointed to a position just before
 * a statement that was among those to be deleted, then after deletion the
 * program counter will point to the first statement beyond those deleted,
 * if such a statement exists, otherwise the program counter will point to
 * the end of the program.
 *
 * @param min  Lower end of the range of line numbers to be deleted.
 * @param max  Upper end of the range of line numbers to be deleted.
 */
int prog_delete(int min, int max) {
    // get the stmt greater than or equal to min
    prog_data *ptr = prog_get_data(min);

    // deletion
    while (1)
    {
        // break if next stmt is head or greater than max
        if (ptr == prog_head || ptr->stmt->lineno > max)
            break;
        // if is counter, counter move to the next stmt
        if (ptr == prog_counter)
            prog_counter = ptr->next;
        // delete statement
        ptr = ptr->next;
        prog_remove_data(ptr->prev);
    }
    
    return 0;
}

/**
 * @brief  Reset the program counter to the beginning of the program.
 * @details  This function resets the program counter to point just
 * before the first statement in the program.
 */
void prog_reset(void) {
    prog_counter = prog_head->next;
}

/**
 * @brief  Fetch the next program statement.
 * @details  This function fetches and returns the first program
 * statement after the current program counter position.  The program
 * counter position is not modified.  The returned pointer should not
 * be used after any subsequent call to prog_delete that deletes the
 * statement from the program store.
 *
 * @return  The first program statement after the current program
 * counter position, if any, otherwise NULL.
 */
STMT *prog_fetch(void) {
    if (prog_counter != prog_head)
        return prog_counter->stmt;
    return NULL;
}

/**
 * @brief  Advance the program counter to the next existing statement.
 * @details  This function advances the program counter by one statement
 * from its original position and returns the statement just after the
 * new position.  The returned pointer should not be used after any
 * subsequent call to prog_delete that deletes the statement from the
 * program store.
 *
 * @return The first program statement after the new program counter
 * position, if any, otherwise NULL.
 */
STMT *prog_next() {
    // if has next
    if (prog_counter != prog_head)
        prog_counter = prog_counter->next;
    
    return prog_fetch();
}

/**
 * @brief  Perform a "go to" operation on the program store.
 * @details  This function performs a "go to" operation on the program
 * store, by resetting the program counter to point to the position just
 * before the statement with the specified line number.
 * The statement pointed at by the new program counter is returned.
 * If there is no statement with the specified line number, then no
 * change is made to the program counter and NULL is returned.
 * Any returned statement should only be regarded as valid as long
 * as no calls to prog_delete are made that delete that statement from
 * the program store.
 *
 * @return  The statement having the specified line number, if such a
 * statement exists, otherwise NULL.
 */
STMT *prog_goto(int lineno) {
    // get first stmt with greter than or equal to lineno
    prog_data *ptr = prog_get_data(lineno);
    // return NULL is no such stmt exists
    if (ptr == prog_head)
        return NULL;
    // if equal, return stmt
    if (ptr->stmt->lineno == lineno) {
        prog_counter = ptr;
        return ptr->stmt;
    }
    // else return NULL
    return NULL;
}

prog_data *prog_get_data(int lineno) {
    // go through the double-linked list and find the fist stmt with lineno greater than or equal to lineno
    prog_data *ptr = prog_head->next;
    while (ptr != prog_head)
    {
        if (ptr->stmt->lineno >= lineno)
            return ptr;
        ptr = ptr->next;
    }
    return prog_head;
}

void prog_remove_data(prog_data* data) {
    data->prev->next = data->next;
    data->next->prev = data->prev;
    if (data->stmt) {
        free_stmt(data->stmt);
    }
    free(data);
}

void prog_init() {
    prog_head = malloc(sizeof(prog_data));
    prog_head->next = prog_head;
    prog_head->prev = prog_head;
    prog_head->stmt = NULL;
    prog_counter = prog_head;
}

void prog_fini() {
    while (prog_head->next != prog_head)
        prog_remove_data(prog_head->next);

    free(prog_head);
    prog_head = NULL;
}