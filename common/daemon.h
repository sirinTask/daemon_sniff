/*
 * daemon.h
 *
*/

#ifndef DAEMON_HEADER
#define DAEMON_HEADER


#include <stddef.h> 

#define DAEMON_DEF_TO_STR_(text) #text
#define DAEMON_DEF_TO_STR(arg) DAEMON_DEF_TO_STR_(arg)


#define DAEMON_MAJOR_VERSION_STR  DAEMON_DEF_TO_STR(DAEMON_MAJOR_VERSION)
#define DAEMON_MINOR_VERSION_STR  DAEMON_DEF_TO_STR(DAEMON_MINOR_VERSION)
#define DAEMON_PATCH_VERSION_STR  DAEMON_DEF_TO_STR(DAEMON_PATCH_VERSION)

#define DAEMON_VERSION_STR  DAEMON_MAJOR_VERSION_STR "." \
                            DAEMON_MINOR_VERSION_STR "." \
                            DAEMON_PATCH_VERSION_STR



struct daemon_info_t
{
    unsigned int terminated     :1;
    unsigned int daemonized     :1;
    unsigned int no_chdir       :1;
    unsigned int no_fork        :1;
    unsigned int no_close_stdio :1;

    const char *pid_file;
    const char *log_file;
    const char *cmd_pipe;

    int        sniff_on_id;
};

extern volatile struct daemon_info_t daemon_info;


int redirect_stdio_to_devnull(void);
int create_pid_file(const char *pid_file_name);


void daemon_error_exit(const char *format, ...);
void exit_if_not_daemonized(int exit_status);

typedef void (*signal_handler_t) (int);

void set_sig_handler(int sig, signal_handler_t handler);

void daemonize2(void (*optional_init)(void *), void *data);
static inline void daemonize() { daemonize2(NULL, NULL); }


#endif 
