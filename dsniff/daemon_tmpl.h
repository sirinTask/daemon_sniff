/*
 * daemon_tmpl.h
 *
*/

#define HAVE_REMOTE
#include<pcap.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<net/ethernet.h>
#include<netinet/ip_icmp.h>	
#include<netinet/udp.h>	
#include<netinet/tcp.h>	
#include<netinet/ip.h>	

#include "daemon.h"
#include "binary_tree.h"



// indexes for long_opt function
enum
{
    cmd_opt_help    = 'h',
    cmd_opt_version = 'v',
    
    //daemon options
    cmd_opt_no_chdir,
    cmd_opt_no_fork,
    cmd_opt_no_close,
    cmd_opt_pid_file,
    cmd_opt_log_file,
    cmd_opt_cmd_pipe,

	cmd_opt_stop,
	cmd_opt_start,
    
    cmd_opt_show_ip_count,
    cmd_opt_select_iface,
    cmd_opt_show_stat,
	cmd_opt_list_reg,
	cmd_enlist_devices

};

//sniffed packets statistic
struct packet_stat 
{
	int tcp;
	int udp;
	int icmp;
	int others;
	int igmp;
	int total;
};

//sniffer lifetime variables
struct sniffer_fields
{
	pcap_if_t *alldevsp;
	pcap_if_t   *device;
	pcap_t      *handle; 								

	char errbuf[100];
	char *devname;
	char devs[100][100];
	int count;
};

struct flow_flags
{
	bool active;
	bool reinit;
};

void process_interaction(struct in_addr);
void process_packet(u_char *, const struct pcap_pkthdr *, const u_char *);
void process_ip_packet(const u_char * , int);
void proc_ip_packet(const u_char * , int);
void proc_tcp_packet(const u_char *  , int );
void proc_udp_packet(const u_char * , int);
void proc_icmp_packet(const u_char * , int );
void* cmd_pipe_thread(void *);
void load_stat(void);
void save_stat(void);
void save_details(void);
void save_stat(void);
