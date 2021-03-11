# Daemon sniffer for linux

#Description
Background running programm that collects statistics on captured traffic packets.
Programm depends on pthread and pcap libs (sudo apt-get install libpcap-dev)

Make handles build of a project, the script located in dsniff subfolder.

Command communication with daemon made using named pipe, separate thread handles it.
Daemon logs results of executed commands in file **log.txt**

After building the project, run it with superuser permissions.
Note Initial Parameters:
       --no_chdir             Don't change the directory to '/'
       --no_fork              Don't do fork
       --no_close             Don't close standart IO files	
       --pid_file [value]     Set pid file name				
       --log_file [value]     Set log file name(not for the output)	  			
       --cmd_pipe [value]     Set CMD Pipe name 
 
User must specify cmd_pipe parameter to further send commands to running daemon via named pipe. The other 5 params may be left untouched, if your intent just to test sniffing abilities.

To run programm use:
    
    sudo ./daemon_tmpl --cmd_pipe Sniffer

That will initialize process tree and create named pipe and log file.

Unfortunatelly time to create client programm or bash scriipt to generate and send commands to daemon I had not enough time - so lets consider using linus standart abilities:

1) To read the output of daemon
    tail -f <path to log.txt>
2) To send commands to a named pipe
    echo '--cmd'> pipe_name &
    
Open another terminal and open log.txt with a tail command as specified above.
From now on, deamon will execute user commands and send results to the file, and tail will display that

Consider the basic flow to test the programm with a following command sequence:
0. Change dir to dsniff
1) Make project via 
  make all
2) Run daemon via 
  sudo ./daemon_tmpl --cmd_pipe Sniffer
3) Check initialization success via 
  ps -o ppid,pid,sid,tty,cmd -C daemon_tmpl
4) Open another terminal and set up displaying output via 
  tail -f /log.txt
5) Request help string form daemon via
  echo '--h'> Sniffer &
6) Start sniffing on default interface via
  echo '--start'> Sniffer &
7) Pause sniffing anytime via
  echo '--stop'> Sniffer &
8) Request info on # of captured packets (persistent through reboots) via
  echo '--stat'> Sniffer &
9) Request list of interactions and it`s packet  statistics via
  echo '--list'> Sniffer & 
10) Request number of captured packets from specified ip
  echo '--show_ip_count 192.168.0.1' > Sniffer &  
12) Switch sniffing interface via (Note! user must pause the sniffing before execution - othervise daemon will ignore switch command)
echo '--stop'> Sniffer &
echo '--select_iface 3' 	    > Sniffer &
13) After switch is made, daemon considers that new sniff session started and clears the content of ip statistic storage. But omits clearing the global stats of # of captured packets (persistent through sessions and reboots)
14) execute commands in any sequence, exept switching interface must be preempted with --stop cmd



     
