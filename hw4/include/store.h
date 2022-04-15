typedef struct store_data
{
    char *name;
    char *value;
    struct store_data *prev;
    struct store_data *next;
} store_data;

store_data *store_head;

void store_init();
store_data* store_get_data(char* var);
void store_remove_data(store_data* data);
void store_add_data(store_data* data);
void store_fini();
char* store_strcpy(char *str);