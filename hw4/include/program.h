typedef struct prog_data
{
    STMT *stmt;
    struct prog_data *prev;
    struct prog_data *next;
} prog_data;

prog_data *prog_head;
prog_data *prog_counter;

void prog_init();
void prog_fini();
prog_data *prog_get_data(int lineno);
void prog_remove_data(prog_data* data);
