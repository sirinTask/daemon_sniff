/*
 * daemon_tmpl.c
 *
*/

#include "daemon_tmpl.h"

struct packet_stat statistics;
struct sockaddr_in 	   source;
struct flow_flags  		flags = {.active = false, .reinit = false};
struct sniffer_fields sn_param;

node *root;
FILE *logfile;
pthread_t  pthread_cmd_pipe;

static const struct option long_opts[] =
{
    { "version",      	no_argument,       NULL, 	cmd_opt_version  	  },
    { "help",         	no_argument,       NULL, 	cmd_opt_help     	  },
    //daemon options
    { "no_chdir",     	no_argument,       NULL,  	cmd_opt_no_chdir 	  },
    { "no_fork" ,     	no_argument,       NULL,   	cmd_opt_no_fork  	  },
    { "no_close",     	no_argument,       NULL,  	cmd_opt_no_close 	  },
    { "pid_file",     	required_argument, NULL,  	cmd_opt_pid_file 	  },
    { "log_file",     	required_argument, NULL,  	cmd_opt_log_file 	  },
    { "cmd_pipe",     	required_argument, NULL,  	cmd_opt_cmd_pipe 	  },
    { "start"   ,     	no_argument,       NULL,  	cmd_opt_start	 	  },
    { "stop"    ,     	no_argument,       NULL,    cmd_opt_stop	 	  },
    { "show_ip_count",	required_argument, NULL,    cmd_opt_show_ip_count },
    { "select_iface", 	required_argument, NULL,   	cmd_opt_select_iface  },
    { "stat ",        	optional_argument, NULL,   	cmd_opt_show_stat  	  },
	{ "list" , 			no_argument, 	   NULL, 	cmd_opt_list_reg	  },
	{ "get_devs",		no_argument, 	   NULL, 	cmd_enlist_devices	  },
    { NULL,           	no_argument,       NULL,  	0 					  }
};

static const char *short_opts = "hv";

static const char *help_str =
        " =========================  Help  =========================\n"
        " Daemon name:  " DAEMON_NAME          "\n"
        " Daemon  ver:  " DAEMON_VERSION_STR   "\n"
#ifdef  DEBUG
        " Build  mode:  debug\n"
#else
        " Build  mode:  release\n"
#endif
        " Build  date:  " __DATE__ "\n"
        " Build  time:  " __TIME__ "\n\n"
        "Configurations on startup :   Description:					  \n\n"
        "       --no_chdir             Don't change the directory to '/'\n"
        "       --no_fork              Don't do fork\n"
        "       --no_close             Don't close standart IO files	\n"
        "       --pid_file [value]     Set pid file name				\n"
        "       --log_file [value]     Set log file name	  			\n"
        "       --cmd_pipe [value]     Set CMD Pipe name 				\n"
		"Commands (via named pipe):										\n"
		""
        "       --start                  	   Start Sniffing	  				\n"
        "       --stop                    	   Stop Sniffing		  			\n"
        "       --show_ip_count [ip_addr] 	   Display # packets recieved for given ip\n" 
        "       --select_iface	[interface_id]     Switch sniffing interface		\n"
        "       --stat                              Statistics of collected packets	\n"
		"       --list                              List registered IP				\n"
        "       --v,  --version           	   Display daemon version			\n"
        "       --h,  --help              	   Display this help			  \n\n";



//load stat from dat.file into structure
void load_stat()
{
    FILE *infile; 
    
    infile = fopen ("Stat.dat", "r"); 
    if (infile == NULL) 
    { 
        fprintf(stderr, "\nNo saved Stat\n"); 
    } 
    else 
    {
          fread(&statistics, sizeof(struct packet_stat), 1, infile);
          fclose (infile); 
    }
    
}

//store structure content into dat.file
void save_stat()
{
    FILE *StoreStat;
    StoreStat = fopen("Stat.dat","w");
	
	    if(logfile==NULL) 
	    {
		    fprintf (stderr,"Unable to create file.");
		    exit(1);
	    }
        fwrite(&statistics, sizeof(struct packet_stat), 1, StoreStat);
        
        fflush(StoreStat);
    
}

//perform actions on recieving sigterm or etc
void daemon_exit_handler(int sig)
{
    save_stat();
    deltree(root);
    fclose(logfile);
    
    unlink(daemon_info.pid_file);
    unlink(daemon_info.cmd_pipe);

    _exit(EXIT_FAILURE);
}

//map incoming signals to handlers
void init_signals(void)
{
    set_sig_handler(SIGINT,  daemon_exit_handler); //for Ctrl-C in terminal (debug)
    set_sig_handler(SIGTERM, daemon_exit_handler);
    set_sig_handler(SIGCHLD, SIG_IGN);
    set_sig_handler(SIGTSTP, SIG_IGN);
    set_sig_handler(SIGTTOU, SIG_IGN);
    set_sig_handler(SIGTTIN, SIG_IGN);
    set_sig_handler(SIGHUP,  SIG_IGN);
}

//create a thread for command pipe and load stored data
void init(void *data)
{
    init_signals();
	load_stat();

    if( pthread_create(&pthread_cmd_pipe, NULL, cmd_pipe_thread, NULL) != 0 )
       daemon_error_exit("Can't create thread_cmd_pipe: %m\n");

}

//Displaying ip tree content in ascending order
void proc_inorder(node * tree)
{
    
    if (tree)
    {
        proc_inorder(tree->left);
        
		fprintf (logfile , "\n   |-Source IP: %20s | %-10d  Packets Recieved" , inet_ntoa(tree->data), tree->transactions );
		
		fflush(logfile);
        proc_inorder(tree->right);
    }
}

// processing the pipe line and commands from the DAEMON_CMD_PIPE
void processing_cmd(int argc, char *argv[])
{
    int opt;
    optind = 0;

    while( (opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1 )
    {
        switch( opt )
        {

            case cmd_opt_help:
                        puts(help_str);
                        exit_if_not_daemonized(EXIT_SUCCESS);
						fprintf (logfile , help_str);
                        break;

            case cmd_opt_version:
                        puts(DAEMON_NAME "  version  " DAEMON_VERSION_STR "\n");
                        exit_if_not_daemonized(EXIT_SUCCESS);
						fprintf (logfile , DAEMON_NAME "  version  " DAEMON_VERSION_STR);
                        break;


                 //daemon options
            case cmd_opt_no_chdir:
                        daemon_info.no_chdir = 1;
                        break;

            case cmd_opt_no_fork:
                        daemon_info.no_fork = 1;
                        break;

            case cmd_opt_no_close:
                        daemon_info.no_close_stdio = 1;
                        break;

            case cmd_opt_pid_file:
                        daemon_info.pid_file = optarg;
                        break;

            case cmd_opt_log_file:
                        daemon_info.log_file = optarg;
                        break;

            case cmd_opt_cmd_pipe:
                        daemon_info.cmd_pipe = optarg;
                        break;

            case cmd_opt_start:
						flags.active = true;
                        break;

            case cmd_opt_stop:
                        flags.active = false;
                        break;

            case cmd_opt_show_ip_count:
                    {
						struct sockaddr_in enclosure;
						inet_aton(optarg, &enclosure.sin_addr);
						node *tmp = search(&root, enclosure.sin_addr);
						
                        if(tmp != NULL)
							fprintf (logfile , "\nRegistered %10d recieved packets from  %-20s\n", tmp->transactions, optarg);
								
						else
							fprintf (logfile , "\nRegistered %10d recieved packets from  %-20s\n", 0, optarg);
								
						fflush(logfile);
                        break;
                    }
			case cmd_opt_list_reg:
                    {
                        fprintf (logfile , "Captured Traffic During Current Session:" );
                        if(root == NULL) 
                                fprintf (logfile , " None\n" );
                            else
                                proc_inorder(root);
                        fprintf (logfile , "\n" );        
                        fflush(logfile);
					    break;
                    }
						
					
			case cmd_enlist_devices:
					{
						/*TODO*/
						break;
					}			
            case cmd_opt_select_iface:
                    {
						if(optarg == NULL)
							daemon_info.sniff_on_id = 1;
						else if(atoi(optarg) >= 0) 
						{
							daemon_info.sniff_on_id = atoi(optarg);
							fprintf (logfile , "Set sniff_on_id:");
							fprintf (logfile ,"%s", optarg);
							fprintf (logfile , "\n");
						}

						fprintf (logfile , "Set reopen flag\n");		
						flags.reinit = true;

                        break;
                    }
            case cmd_opt_show_stat:
                        {
							fprintf(logfile, "\nTOTAL FOR ALL SESSIONS: TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\n\n", statistics.tcp , statistics.udp , statistics.icmp , statistics.igmp , statistics.others , statistics.total);
							fflush(logfile);
                            break;
                        }            
   
            default:
                        puts("for more detail see help\n\n");
                        exit_if_not_daemonized(EXIT_FAILURE);
                        break;
        }
    }
}

//initialize and handle DAEMON_CMD_PIPE
void* cmd_pipe_thread(void *thread_arg)
{
    int   fd;
    int   argc;
    char *arg;
    char **argv;
    char *cmd_pipe_buf;



    pthread_detach(pthread_self());
    unlink(daemon_info.cmd_pipe);

	
    argv = (char **)malloc(PIPE_BUF*sizeof(char *));
    if( !argv )
        daemon_error_exit("Can't get mem for argv (CMD_PIPE): %m\n");


    cmd_pipe_buf = (char *)malloc(PIPE_BUF);
    if( !cmd_pipe_buf )
        daemon_error_exit("Can't get mem for cmd_pipe_buf: %m\n");


    if( mkfifo(daemon_info.cmd_pipe, 0622) != 0 )
        daemon_error_exit("Can't create CMD_PIPE: %m\n");


    fd = open(daemon_info.cmd_pipe, O_RDWR);
    if( fd == -1 )
        daemon_error_exit("Can't open CMD_PIPE: %m\n");

    while(1)
    {
        memset(cmd_pipe_buf, 0, PIPE_BUF);


        if( read(fd, cmd_pipe_buf, PIPE_BUF) == -1 )             // wait for command from DAEMON_CMD_PIPE
            daemon_error_exit("read CMD_PIPE return -1: %m\n");


        argc = 1;                                                // see getopt_long function
        arg  = strtok(cmd_pipe_buf, " \t\n");


        while( (arg != NULL)  && (argc < PIPE_BUF) )
        {
            argv[argc++] = arg;
            arg          = strtok(NULL, " \t\n");
        }


        if( argc > 1 )
          processing_cmd(argc, argv);                            //handle command
    }


    return NULL;
}

//register processed packet onto session tree
void process_interaction(struct in_addr src)
{
	node *tmp = search(&root, src);
	
	if(tmp)
		tmp->transactions++;
	else 
		insert(&root, src);
}

//Function switches handlers based on captured packet`s protocol
//Allows further expansion on daemon`s functionality
void process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *buffer)
{
	int size = header->len;

	struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));					//Exclude ethernet header
	statistics.total++;
	switch (iph->protocol) 																	//Switch on protocols to collect general stat
	{
		case 1:  //ICMP Protocol
			statistics.icmp++;
			proc_icmp_packet( buffer , size);
			break;
		
		case 2:  //IGMP Protocol
			statistics.igmp++;
			break;
		
		case 6:  //TCP Protocol
			statistics.tcp++;
			proc_tcp_packet(buffer , size);
			break;
		
		case 17: //UDP Protocol
			statistics.udp++;
			proc_udp_packet(buffer , size);
			break;
		
		default: // ARP etc.
			statistics.others++;
			break;
	}
}

void proc_ip_header(const u_char * Buffer, int Size)
{
    
	struct iphdr *iph = (struct iphdr *)(Buffer  + sizeof(struct ethhdr) );

	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = iph->saddr;
	
		process_interaction(source.sin_addr);
}

void proc_tcp_packet(const u_char * Buffer, int Size)
{
    /*available for logging extension*/
	proc_ip_header(Buffer,Size);	
}

void proc_udp_packet(const u_char *Buffer , int Size)
{
    /*available for logging extension*/
	proc_ip_header(Buffer,Size);			
}

void proc_icmp_packet(const u_char * Buffer , int Size)
{
    /*available for logging extension*/
	proc_ip_header(Buffer , Size);
}


//open file for output writes during operation
void configure_logfile(void)
{
    logfile = fopen("log.txt","w");
	
	if(logfile==NULL) 
	{
		fprintf (stderr,"Unable to create file.");
		exit(1);
	}
    
}

//gather devices info and create/update handler based on current sniff_on_id
void configure_handler(void)
{
    if(pcap_findalldevs( &sn_param.alldevsp, sn_param.errbuf) )
	{
		fprintf (stderr,"Unable to perform device lookup.");
		exit(1);
	}
	
	fprintf (logfile,"\nGathering available DevInfo :\n");
	for(sn_param.device = sn_param.alldevsp, sn_param.count = 1 ; sn_param.device != NULL ; sn_param.device = sn_param.device->next)
	{
		fprintf (logfile, "%d. %s - %s\n" , sn_param.count , sn_param.device->name , sn_param.device->description);
		if(sn_param.device->name != NULL)
			strcpy(sn_param.devs[sn_param.count] , sn_param.device->name);
		
		sn_param.count++;
	}
	fprintf (logfile,"\n^Perform sniffing interface on id: %d\n", daemon_info.sniff_on_id );

	sn_param.handle = pcap_open_live(sn_param.devs[daemon_info.sniff_on_id], 65536 , 1 , 0 , sn_param.errbuf);
	
	if (sn_param.handle == NULL) 
	{
		fprintf (stderr, "Couldn't open device %s : %s\n" , sn_param.devname , sn_param.errbuf);
		exit(1);
	}
	pcap_setnonblock(sn_param.handle, -1 , sn_param.errbuf);
}

int main(int argc, char *argv[])
{
   	
    processing_cmd(argc, argv);
    daemonize2(init, NULL);

	configure_logfile();    //open file for reactions to user commands
	configure_handler();    //setup sniffing handler

    while( !daemon_info.terminated )                                        //while daemon alive
    {
        if(flags.active){                                                   //perform non-blocking sniffing     
            pcap_dispatch(sn_param.handle , -1 , process_packet , NULL);
        }
		if(flags.reinit)                                                    //reconfigure handler
		{
            configure_handler();
			deltree(root->left);
            deltree(root->right);
            root = NULL;
            //deltree(root->right);
            
			flags.reinit = false;
		}

        fflush(logfile); 
        sleep(1);
    }
    
}
