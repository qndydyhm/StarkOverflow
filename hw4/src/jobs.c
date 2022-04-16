#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

#include "mush.h"
#include "debug.h"
#include "store.h"
#include "program.h"
#include "jobs.h"

/*
 * This is the "jobs" module for Mush.
 * It maintains a table of jobs in various stages of execution, and it
 * provides functions for manipulating jobs.
 * Each job contains a pipeline, which is used to initialize the processes,
 * pipelines, and redirections that make up the job.
 * Each job has a job ID, which is an integer value that is used to identify
 * that job when calling the various job manipulation functions.
 *
 * At any given time, a job will have one of the following status values:
 * "new", "running", "completed", "aborted", "canceled".
 * A newly created job starts out in with status "new".
 * It changes to status "running" when the processes that make up the pipeline
 * for that job have been created.
 * A running job becomes "completed" at such time as all the processes in its
 * pipeline have terminated successfully.
 * A running job becomes "aborted" if the last process in its pipeline terminates
 * with a signal that is not the result of the pipeline having been canceled.
 * A running job becomes "canceled" if the jobs_cancel() function was called
 * to cancel it and in addition the last process in the pipeline subsequently
 * terminated with signal SIGKILL.
 *
 * In general, there will be other state information stored for each job,
 * as required by the implementation of the various functions in this module.
 */

/**
 * @brief  Initialize the jobs module.
 * @details  This function is used to initialize the jobs module.
 * It must be called exactly once, before any other functions of this
 * module are called.
 *
 * @return 0 if initialization is successful, otherwise -1.
 */
int jobs_init(void) {
    store_init();
    prog_init();
    for (size_t i = 0; i < MAX_JOBS; i++)
    {
        job_data_array[i] = NULL;
    }
    job_current_size = 0;
    signal(SIGCHLD, &chld_handler);
    signal(SIGIO, &io_handler);
    return 0;
}

void chld_handler(int sig) {
    pid_t pid;
    int   status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (size_t i = 0; i < MAX_JOBS; i++)
        {
            if (job_data_array[i])
            {
                if (job_data_array[i]->pid == pid)
                {
                    if (WIFEXITED(status))
                    {
                        job_data_array[i]->exit_status = WEXITSTATUS(status);
                        job_data_array[i]->status = COMPLETED;
                    }
                    else {
                        job_data_array[i]->exit_status = 6;
                        job_data_array[i]->status = ABORTED;
                    }
                }
            }
        }
    }
}

void io_handler(int sig) {
    for (size_t i = 0; i < MAX_JOBS; i++)
    {
        if (job_data_array[i])
        {
            if (job_data_array[i]->pipeline->capture_output)
            {
                char c;
                while (1)
                {
                    int num = read(job_data_array[i]->pipe[0], &c, sizeof(char));
                    if (num == 0 || num == -1) {
                        break;
                    }
                    job_data_array[i]->output_length ++;
                    job_data_array[i]->output = realloc(job_data_array[i]->output, (job_data_array[i]->output_length + 1) * sizeof(char));
                    job_data_array[i]->output[job_data_array[i]->output_length - 1] = c;
                    job_data_array[i]->output[job_data_array[i]->output_length] = '\0';
                }
            }
        }
    }
}

/**
 * @brief  Finalize the jobs module.
 * @details  This function is used to finalize the jobs module.
 * It must be called exactly once when job processing is to be terminated,
 * before the program exits.  It should cancel all jobs that have not
 * yet terminated, wait for jobs that have been cancelled to terminate,
 * and then expunge all jobs before returning.
 *
 * @return 0 if finalization is completely successful, otherwise -1.
 */
int jobs_fini(void) {
    store_fini();
    prog_fini();
    for (size_t i = 0; i < MAX_JOBS; i++)
    {
        if (job_data_array[i])
        {
            if (job_data_array[i]->status != COMPLETED &&
                job_data_array[i]->status != ABORTED &&
                job_data_array[i]->status != CANCELED)
                if (jobs_cancel(i) == -1)
                    return -1;
            if (jobs_expunge(i) == -1)
                return -1;
        }
        
    }
    return 0;
}

/**
 * @brief  Print the current jobs table.
 * @details  This function is used to print the current contents of the jobs
 * table to a specified output stream.  The output should consist of one line
 * per existing job.  Each line should have the following format:
 *
 *    <jobid>\t<pgid>\t<status>\t<pipeline>
 *
 * where <jobid> is the numeric job ID of the job, <status> is one of the
 * following strings: "new", "running", "completed", "aborted", or "canceled",
 * and <pipeline> is the job's pipeline, as printed by function show_pipeline()
 * in the syntax module.  The \t stand for TAB characters.
 *
 * @param file  The output stream to which the job table is to be printed.
 * @return 0  If the jobs table was successfully printed, -1 otherwise.
 */
int jobs_show(FILE *file) {
    for (size_t i = 0; i < MAX_JOBS; i++)
    {
        if (job_data_array[i])
        {
            char *status;
            switch (job_data_array[i]->status)
            {
            case NEW:
                status = "new";
                break;
            case RUNNING:
                status = "running";
                break;
            case COMPLETED:
                status = "completed";
                break;
            case ABORTED:
                status = "aborted";
                break;
            case CANCELED:
                status = "canceled";
                break;
            default:
                break;
            }
            fprintf(file, "%ld\t%d\t%s", i, job_data_array[i]->pid, status);
            fprintf(file, "\t");
            show_pipeline(file, job_data_array[i]->pipeline);
            fprintf(file, "\n");
        }
    }
    return 0;
}

/**
 * @brief  Create a new job to run a pipeline.
 * @details  This function creates a new job and starts it running a specified
 * pipeline.  The pipeline will consist of a "leader" process, which is the direct
 * child of the process that calls this function, plus one child of the leader
 * process to run each command in the pipeline.  All processes in the pipeline
 * should have a process group ID that is equal to the process ID of the leader.
 * The leader process should wait for all of its children to terminate before
 * terminating itself.  The leader should return the exit status of the process
 * running the last command in the pipeline as its own exit status, if that
 * process terminated normally.  If the last process terminated with a signal,
 * then the leader should terminate via SIGABRT.
 *
 * If the "capture_output" flag is set for the pipeline, then the standard output
 * of the last process in the pipeline should be redirected to be the same as
 * the standard output of the pipeline leader, and this output should go via a
 * pipe to the main Mush process, where it should be read and saved in the data
 * store as the value of a variable, as described in the assignment handout.
 * If "capture_output" is not set for the pipeline, but "output_file" is non-NULL,
 * then the standard output of the last process in the pipeline should be redirected
 * to the specified output file.   If "input_file" is set for the pipeline, then
 * the standard input of the process running the first command in the pipeline should
 * be redirected from the specified input file.
 *
 * @param pline  The pipeline to be run.  The jobs module expects this object
 * to be valid for as long as it requires, and it expects to be able to free this
 * object when it is finished with it.  This means that the caller should not pass
 * a pipeline object that is shared with any other data structure, but rather should
 * make a copy to be passed to this function.
 * 
 * @return  -1 if the pipeline could not be initialized properly, otherwise the
 * value returned is the job ID assigned to the pipeline.
 */
int jobs_run(PIPELINE *pline) {
    if (!pline)
        return -1;
    
    pline = copy_pipeline(pline);
    // return -1 if no place for new job
    if (job_current_size + 1 >= MAX_JOBS)
        return -1;
    
    // find index of first empty job
    int index = 0;
    while (job_data_array[index])
        index++;

    // create new job and put it in job array
    job_data *new = malloc(sizeof(job_data));
    new->status = NEW;
    new->pipeline = pline;
    new->output = NULL;
    new->output_length = 0;
    job_data_array[index] = new;

    // create a pipeline if capture_output is set
    if (pline->capture_output)
    {
        if (pipe(new->pipe) == -1)
            return -1;
        fcntl(new->pipe[0], F_SETFL, O_NONBLOCK);
        fcntl(new->pipe[0], F_SETFL, O_ASYNC);
        fcntl(new->pipe[0], __F_SETOWN, getpid());
        new->output = malloc(sizeof(char));
        new->output[0] = '\0';
    }

    // fork a leader process
    new->pid = fork();
    if (new->pid == -1)
        return -1;

    if (new->pid == 0)
    {
        // Leader process
        // calculate the length of pipeline
        COMMAND *ptr = pline->commands;
        size_t size = 0;
        while (ptr)
        {
            ptr = ptr->next;
            size++;
        }
        // create array of pids and pipes
        int pids[size];
        int fds[size - 1][2];
        for (size_t i = 0; i < size - 1; i++)
        {
            if (pipe(fds[i]) == -1)
                return -1;
        }
        
        // reset ptr to the beginning of pipeline
        ptr = pline->commands;
        for (size_t i = 0; i < size; i++)
        {
            pids[i] = fork();
            if(pids[i] == -1) {
                return -1;
            }
            if (pids[i] == 0)
            {
                // is a child process
                // redirect input and output

                // if is the first one and input file is defined
                // redirect stdin to file
                if (i == 0 && pline->input_file)
                {
                    int inputfd;
                    if ((inputfd = open(pline->input_file, O_RDONLY)) == -1)
                        return -1;
                    if (dup2(inputfd, STDIN_FILENO) == -1)
                        return -1;
                    close(inputfd);
                }
                // if is the last one and capture is defined
                // redirect stdout to file
                if (i == size - 1 && pline->capture_output)
                {
                    if(dup2(new->pipe[1], STDOUT_FILENO) == -1)
                        return -1;
                    close(new->pipe[1]);
                    close(new->pipe[0]);
                }
                // if is the last one and output file is defined
                // redirect stdout to file
                if (i == size - 1 && pline->output_file)
                {
                    int outputfd;
                    if ((outputfd = open(pline->output_file, O_WRONLY | O_CREAT, 0666)) == -1)
                        return -1;
                    if(dup2(outputfd, STDOUT_FILENO) == -1)
                        return -1;
                    close(outputfd);
                }
                // if not first one
                // redirect input
                if (i != 0)
                    if (dup2(fds[i-1][0], STDIN_FILENO) == -1)
                        return -1;
                // if not last one
                // redirect output
                if (i != size - 1)
                    if (dup2(fds[i][1], STDOUT_FILENO) == -1)
                        return -1;
                // close all fds
                for (size_t fd = 0; fd < size - 1; fd++)
                {
                    close(fds[fd][0]);
                    close(fds[fd][1]);
                }

                // get args
                char **args = jobs_get_args(ptr->args);
                
                // run command
                if (execvp(args[0], args) == -1) {
                    char **char_ptr = args;
                    while (*char_ptr)
                    {
                        free(*char_ptr);
                        char_ptr ++;
                    }
                    free(args);
                    int exit_code = errno;
                    close(STDIN_FILENO);
                    close(STDOUT_FILENO);
                    exit(exit_code);
                }
            }
            
            else {
                // Leader process
                ptr = ptr->next;
            }
        }
        
        // close fds for Leader
        for (size_t i = 0; i < size - 1; i++)
        {
            close(fds[i][0]);
            close(fds[i][1]);
        }
        
        int exit_code = 0;
        // wait for childs
        for (size_t i = 0; i < size; i++)
        {
            int status;
            waitpid(pids[i], &status, 0);
            if (WIFEXITED(status))
                exit_code = WEXITSTATUS(status);
        }
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        exit(exit_code);
    }
    
    if (new->pipeline->capture_output)
        close(new->pipe[1]);
    new->status = RUNNING;
    
    return index;
}

char **jobs_get_args(ARG *args) {
    // get length of arg
    int arg_size = 0;
    ARG *argptr = args;
    while (argptr)
    {
        arg_size ++;
        argptr = argptr->next;
    }
    argptr = args;
    // copy args into result
    char **result = malloc((arg_size + 1)*sizeof(char*));
    for (size_t i = 0; i < arg_size; i++)
    {
        result[i] = eval_to_string(argptr->expr);
        argptr = argptr->next;
    }
    result[arg_size] = NULL;
    
    return result;
}

/**
 * @brief  Wait for a job to terminate.
 * @details  This function is used to wait for the job with a specified job ID
 * to terminate.  A job has terminated when it has entered the COMPLETED, ABORTED,
 * or CANCELED state.
 *
 * @param  jobid  The job ID of the job to wait for.
 * @return  the exit status of the job leader, as returned by waitpid(),
 * or -1 if any error occurs that makes it impossible to wait for the specified job.
 */
int jobs_wait(int jobid) {
    if (jobid >= MAX_JOBS || jobid < 0)
        return -1;
    if (!job_data_array[jobid])
        return -1;
    if (job_data_array[jobid]->status == COMPLETED ||
        job_data_array[jobid]->status == ABORTED ||
        job_data_array[jobid]->status == CANCELED)
        return job_data_array[jobid]->exit_status;
    int status;
    if (waitpid(job_data_array[jobid]->pid, &status, 0) == -1)
        return -1;
    int return_stat = -1;
    if (WIFEXITED(status)) {
        return_stat = WEXITSTATUS(status);
        job_data_array[jobid]->status = COMPLETED;
        job_data_array[jobid]->exit_status = return_stat;
    }
    else {
        job_data_array[jobid]->status = ABORTED;
        job_data_array[jobid]->exit_status = 6;
    }
    return return_stat;
}

/**
 * @brief  Poll to find out if a job has terminated.
 * @details  This function is used to poll whether the job with the specified ID
 * has terminated.  This is similar to jobs_wait(), except that this function returns
 * immediately without waiting if the job has not yet terminated.
 *
 * @param  jobid  The job ID of the job to wait for.
 * @return  the exit status of the job leader, as returned by waitpid(), if the job
 * has terminated, or -1 if the job has not yet terminated or if any other error occurs.
 */
int jobs_poll(int jobid) {
    if (jobid >= MAX_JOBS || jobid < 0)
        return -1;
    if (!job_data_array[jobid])
        return -1;
    if (job_data_array[jobid]->status != COMPLETED &&
        job_data_array[jobid]->status != ABORTED &&
        job_data_array[jobid]->status != CANCELED)
        return -1;
    
    return job_data_array[jobid]->exit_status;
}

/**
 * @brief  Expunge a terminated job from the jobs table.
 * @details  This function is used to expunge (remove) a job that has terminated from
 * the jobs table, so that space in the table can be used to start some new job.
 * In order to be expunged, a job must have terminated; if an attempt is made to expunge
 * a job that has not yet terminated, it is an error.  Any resources (exit status,
 * open pipes, captured output, etc.) that were being used by the job are finalized
 * and/or freed and will no longer be available.
 *
 * @param  jobid  The job ID of the job to expunge.
 * @return  0 if the job was successfully expunged, -1 if the job could not be expunged.
 */
int jobs_expunge(int jobid) {
    if (jobid >= MAX_JOBS || jobid < 0)
        return -1;
    if (!job_data_array[jobid])
        return -1;
    if (job_data_array[jobid]->status != COMPLETED &&
        job_data_array[jobid]->status != ABORTED &&
        job_data_array[jobid]->status != CANCELED)
        return -1;
    
    
    if (job_data_array[jobid]->output)
        free(job_data_array[jobid]->output);

    if (job_data_array[jobid]->pipeline->capture_output) {
        close(job_data_array[jobid]->pipe[0]);
    }
    
    free_pipeline(job_data_array[jobid]->pipeline);
    job_current_size -= 1;
    free(job_data_array[jobid]);
    job_data_array[jobid] = NULL;

    return 0;
}

/**
 * @brief  Attempt to cancel a job.
 * @details  This function is used to attempt to cancel a running job.
 * In order to be canceled, the job must not yet have terminated and there
 * must not have been any previous attempt to cancel the job.
 * Cancellation is attempted by sending SIGKILL to the process group associated
 * with the job.  Cancellation becomes successful, and the job actually enters the canceled
 * state, at such subsequent time as the job leader terminates as a result of SIGKILL.
 * If after attempting cancellation, the job leader terminates other than as a result
 * of SIGKILL, then cancellation is not successful and the state of the job is either
 * COMPLETED or ABORTED, depending on how the job leader terminated.
 *
 * @param  jobid  The job ID of the job to cancel.
 * @return  0 if cancellation was successfully initiated, -1 if the job was already
 * terminated, a previous attempt had been made to cancel the job, or any other
 * error occurred.
 */
int jobs_cancel(int jobid) {
    if (jobid >= MAX_JOBS || jobid < 0)
        return -1;
    if (!job_data_array[jobid])
        return -1;
    if (job_data_array[jobid]->status == COMPLETED ||
        job_data_array[jobid]->status == ABORTED ||
        job_data_array[jobid]->status == CANCELED)
        return -1;

    int pid = job_data_array[jobid]->pid;
    if (kill(pid, SIGKILL) == -1)
        return -1;
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        job_data_array[jobid]->exit_status = WEXITSTATUS(status);
    }
    else
    {
        job_data_array[jobid]->exit_status = 9;
    }
    job_data_array[jobid]->status = CANCELED;
    return 0;
}

/**
 * @brief  Get the captured output of a job.
 * @details  This function is used to retrieve output that was captured from a job
 * that has terminated, but that has not yet been expunged.  Output is captured for a job
 * when the "capture_output" flag is set for its pipeline.
 *
 * @param  jobid  The job ID of the job for which captured output is to be retrieved.
 * @return  The captured output, if the job has terminated and there is captured
 * output available, otherwise NULL.
 */
char *jobs_get_output(int jobid) {
    if (jobid >= MAX_JOBS || jobid < 0)
        return NULL;
    if (!job_data_array[jobid])
        return NULL;
    if (job_data_array[jobid]->status != COMPLETED &&
        job_data_array[jobid]->status != ABORTED &&
        job_data_array[jobid]->status != CANCELED)
        return NULL;
    if (!job_data_array[jobid]->pipeline->capture_output)
        return NULL;
    if (!job_data_array[jobid]->output)
        return "";
    return job_data_array[jobid]->output;
}

/**
 * @brief  Pause waiting for a signal indicating a potential job status change.
 * @details  When this function is called it blocks until some signal has been
 * received, at which point the function returns.  It is used to wait for a
 * potential job status change without consuming excessive amounts of CPU time.
 *
 * @return -1 if any error occurred, 0 otherwise.
 */
int jobs_pause(void) {
    if (wait(NULL) == -1)
        return -1; 
    return 0;
}
