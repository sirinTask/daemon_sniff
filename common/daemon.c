/*
 * daemon.c
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>


#include "daemon.h"


//initialize parameters and flags for daemon operation
volatile struct daemon_info_t daemon_info =
{
    .terminated = 0,
    .daemonized = 0,                   //custom flag will be set in finale function daemonize()


    #ifdef  DAEMON_NO_CHDIR
        .no_chdir = DAEMON_NO_CHDIR,
    #else
        .no_chdir = 0,
    #endif


    #ifdef  DAEMON_NO_FORK
        .no_fork = DAEMON_NO_FORK,
    #else
        .no_fork = 0,
    #endif


    #ifdef  DAEMON_NO_CLOSE_STDIO
        .no_close_stdio = DAEMON_NO_CLOSE_STDIO,
    #else
        .no_close_stdio = 0,
    #endif

   
    #ifdef  DAEMON_PID_FILE_NAME
        .pid_file = DAEMON_PID_FILE_NAME,
    #else
        .pid_file = NULL,
    #endif


    #ifdef  DAEMON_LOG_FILE_NAME
        .log_file = DAEMON_LOG_FILE_NAME,
    #else
        .log_file = NULL,
    #endif


    #ifdef  DAEMON_CMD_PIPE_NAME
        .cmd_pipe = DAEMON_CMD_PIPE_NAME,
    #else
        .cmd_pipe = NULL,
    #endif

    #ifdef  DAEMON_CMD_DEV_ID
        .sniff_on_id = DAEMON_CMD_PIPE_NAME,
    #else
        .sniff_on_id = 1,
    #endif

};

//exit if daemonization flag wasnt set after daemonization
void exit_if_not_daemonized(int exit_status)
{
    if( !daemon_info.daemonized )
        _exit(exit_status);
}


//log err message on failure
void daemon_error_exit(const char *format, ...)
{
    va_list ap;


    if( format &&  *format )
    {
        va_start(ap, format);
        fprintf(stderr, "%s: ", DAEMON_NAME);
        vfprintf(stderr, format, ap);
        va_end(ap);
    }


    _exit(EXIT_FAILURE);
}


//perform switching output logging
int redirect_stdio_to_devnull(void)
{
    int fd;


    fd = open("/dev/null", O_RDWR);
    if(fd == -1)
        return -1; //error can't open file


    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);


    if(fd > 2)
        close(fd);


    return 0; //complete
}


//handle proper pid file creation
int create_pid_file(const char *pid_file_name)
{
    int fd;
    const int BUF_SIZE = 32;
    char buf[BUF_SIZE];



    if( !pid_file_name )
    {
        errno = EINVAL;
        return -1;
    }


    fd = open(pid_file_name, O_RDWR | O_CREAT, 0644);
    if(fd == -1)
        return -1; // Could not create on PID file


    if( lockf(fd, F_TLOCK, 0) == -1 )
    {
        close(fd);
        return -1; // Could not get lock on PID file
    }


    if( ftruncate(fd, 0) != 0 )
    {
        close(fd);
        return -1; // Could not truncate on PID file
    }


    snprintf(buf, BUF_SIZE, "%ld\n", (long)getpid());
    if( write(fd, buf, strlen(buf)) != (int)strlen(buf) )
    {
        close(fd);
        return -1; // Could not write PID to PID file
    }


    return fd; //good job
}



//initializing and testing signals
void set_sig_handler(int sig, signal_handler_t handler)
{
    if(signal(sig, handler) == SIG_ERR)
        daemon_error_exit("Can't set handler for signal: %d %m\n", sig);
}


//perform forking
static void do_fork()
{
    switch( fork() )                                     // Become background process
    {
        case -1:  daemon_error_exit("Can't fork: %m\n");
        case  0:  break;                                 // child process (go next)
        default:  _exit(EXIT_SUCCESS);                   // We can exit the parent process
    }

    // ---- At this point we are executing as the child process ----
}


//process deamonization and handle failures
void daemonize2(void (*optional_init)(void *), void *data)
{
    if( !daemon_info.no_fork )
        do_fork();


    // Reset the file mode mask
    umask(0);


    // Create a new process group(session) (SID) for the child process
    // call setsid() only if fork is done
    if( !daemon_info.no_fork && (setsid() == -1) )
        daemon_error_exit("Can't setsid: %m\n");


    // Change the current working directory to "/"
    // This prevents the current directory from locked
    // The demon must always change the directory to "/"
    if( !daemon_info.no_chdir && (chdir("/") != 0) )
        daemon_error_exit("Can't chdir: %m\n");


    if( daemon_info.pid_file && (create_pid_file(daemon_info.pid_file) == -1) )
        daemon_error_exit("Can't create pid file: %s: %m\n", daemon_info.pid_file);


    // call user functions for the optional initialization
    // before closing the standardIO (STDIN, STDOUT, STDERR)
    if( optional_init )
        optional_init(data);


    if( !daemon_info.no_close_stdio && (redirect_stdio_to_devnull() != 0) )
        daemon_error_exit("Can't redirect stdio to /dev/null: %m\n");


    daemon_info.daemonized = 1; //success
}
