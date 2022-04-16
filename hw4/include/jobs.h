typedef enum {
    NEW,
    RUNNING,
    COMPLETED,
    ABORTED,
    CANCELED,
} JOB_STATUS;

typedef struct job_data {
    JOB_STATUS status;
    int pid;
    PIPELINE *pipeline;
    int exit_status;
    char *output;
    int output_length;
    int pipe[2];
} job_data;

job_data *job_data_array[MAX_JOBS];
int job_current_size;
char **jobs_get_args(ARG *args);
void chld_handler(int sig);
void io_handler(int sig);