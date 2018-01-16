#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define	MY_PORT	2225

FILE * fp;
FILE * flog;
int entry_len = 0;
char ** whiteboard;

char message[1001];
char line[10001];
	
int	sock, snew, fromlength, number, outnum;
struct	sockaddr_in	master, from;

pthread_mutex_t mutex;
pthread_t * thread;
int tnum = 20, actual_tnum = 0, id[10001];

void sigTermHandler(int sig){
	int i;
	fp = fopen("whiteboard.all","w");
	for (i=1;i<entry_len+1;i++)
		fprintf(fp,"%s", whiteboard[i]);
	free(whiteboard);
	fclose(fp);
	fclose(flog);
	exit(0);
}

void get_entry_len (char * start_way, char * info){ 

	char line[10001];

	// -f: read the information from the statfile and get the number of entries
    if (!strcmp(start_way, "-f")){	
        char * filename = info;
        fp = fopen(filename, "r");
        if (fp == NULL)
            exit(EXIT_FAILURE);

        while (fgets(line, sizeof(line), fp)) 
            entry_len ++;
        entry_len =  (entry_len / 2);
		fclose(fp);
    }

	// -n: get the number of entries and create a statfile with the initial message
    if (!strcmp(start_way, "-n")){
        entry_len = strtol(info, NULL, 10);
		fp = fopen("whiteboard.all","w");
		int i = 0;
		for (i=0;i<entry_len;i++){
			fprintf(fp,"!%dp0\n\n", i+1);
		}
		fclose(fp);
    }
}

void * thread_body(){
	
	int i = 0;
	//welcome message
	char * welcome;
	welcome = malloc(sizeof("CMPUT379 Whiteboard Server v0\n00\n"));
	 //put the welcome message and the length of the entries together
	sprintf(welcome, "CMPUT379 Whiteboard Server v0\n%d\n", entry_len);
	send(snew, welcome, strlen(welcome), 0);

	bzero(message,strlen(message));
	bzero(welcome,strlen(welcome));
	free(welcome);

	
	recv(snew, message, 10001, 0);

	int entry = 0, check = 0;
	int error = 0;
	int rest = 0;
	// extract the number of the entry

	if (message[0] != '@' && message[0]!= '?'){
		char symbol = message[0];
		sprintf(message, "!%ce13\nUnknown Mark!\n",symbol);
		error = 1;
		send(snew, message, strlen(message), 0);
		//unlock after done all the operations
		pthread_mutex_unlock(&mutex);
		//printf("%s", message);	
	}
	else{
		for (i = 1; i < strlen(message)-1; i++){
			if (message[i] >= '0' && message[i] <= '9')
				entry = entry*10 +  (message[i] - '0');
			//else error 
			else {
				if (message[i] == '\n' || message[i] == 'p' || message[i] == 'c'){
					rest = i; 
					break;
				}
				else{
					error = 1;
					char emessage[1001];
					int j = 1;
					emessage[0] = '!';
					for (j = 1; j< strlen(message);j++){
						if (message[j] != '\n' )
							emessage[j-1] = message[j];
						else
							break;
					}
					bzero(message,strlen(message));
					sprintf(message, "!%s", emessage);
					strcat(message, "e20\nInvalid Entry Input!\n");
					send(snew, message, strlen(message), 0);
					//printf("%s", message);
					pthread_mutex_unlock(&mutex);
					break;
				}
			}
		}
	}
	if (error == 0){
	//Check the information in the entry 
		if (message[0] == '?'){
			bzero(message,strlen(message));
			if (entry > entry_len || entry < 1) 
				sprintf(message, "!%de14\nNo such entry!\n", entry);
			else
				strcpy(message, whiteboard[entry]);
		}

			
		else if (message[0] == '@'){
			// lock when handling message
			pthread_mutex_lock(&mutex);

			//bzero(message,strlen(message));
			char new[10001], type = message[rest];
			int actual_len = 0, bcount = 0;
	
			strcpy(new, message);
			new[0] = '!';

			int str_len = 0;
			for (i = rest+1;i < strlen(message)-1;i++){
				if (message[i] >= '0' && message[i] <= '9' && bcount != 1){
					str_len = str_len*10 + (message[i] - '0');
				}
				if (bcount == 1)
					actual_len ++;
				if (message[i] == '\n')
					bcount = 1;
			}
			//printf("%d %d\n",str_len, actual_len);
			if(str_len != actual_len){
				sprintf(message, "!%de26\nString Length Not Matching\n", entry);
			}
			else{
				//printf("%s", new);
				memset(whiteboard[entry],0,strlen(whiteboard[entry]));
				strcpy(whiteboard[entry], new);
				// Successfully update confirmation message
				sprintf(message, "!%de0\n\n", entry);
				//printf("%s", message);
			}

		}
			
		send(snew, message, strlen(message), 0);
		//unlock after done all the operations
		pthread_mutex_unlock(&mutex);
		//printf("%s", message);	
	}
	
	close(snew);
	number++;
	
	return ;
}

int main(int argc, char * argv[])
{
	/*
	// Daemon Process
	pid_t pid = 0;
    pid_t sid = 0;

    pid = fork();

    if (pid < 0)
    {
        printf("fork failed!\n");
        exit(1);
    }

    if (pid > 0)
    {
    	// in the parent
       printf("pid of child process %d \n", pid);
       exit(0);
    }

    umask(0);

	// open a log file
    flog = fopen ("logfile.log", "w+");
    if(!fp){
    	printf("cannot open log file");
    }

    // create new process group -- don't want to look like an orphan
    sid = setsid();
    if(sid < 0)
    {
    	fprintf(fp, "cannot create new process group");
        exit(1);
    }

    // Change the current working directory 
    if ((chdir("/")) < 0) {
      printf("Could not change working directory to /\n");
      exit(1);
    }

	// close standard fds
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
*/
//--------------------------------------------------------------------------------------------------------------------

	struct sigaction saSigTerm;

    saSigTerm.sa_handler = sigTermHandler;
    sigemptyset(&saSigTerm.sa_mask);
    sigaction(SIGTERM,&saSigTerm,0);

	pthread_mutex_init(&mutex,NULL);
	thread = malloc(tnum * sizeof(pthread_t));

	get_entry_len(argv[2], argv[3]);	// getting the lentgh of the entries

	int i = 0;

	whiteboard = malloc ((entry_len+1) * sizeof(char *));
	for (i = 0; i < entry_len+1; i++)
		whiteboard[i] = malloc (1001 * sizeof (char *));
	for (i = 0;i < 10001; i++)
		id[i] = i;

	//Get the value into whiteboard array
	i = 1;

	if (!strcmp(argv[2], "-f"))	fp = fopen (argv[3], "r+");
	if (!strcmp(argv[2], "-n"))	fp = fopen ("whiteboard.all","r+");
	while (fgets(line, sizeof(line), fp)!=NULL){
		strcpy(whiteboard[i], line);
		fgets(line, sizeof(line), fp);
		strcat(whiteboard[i], line);
		i++;
	}
	fclose(fp);

	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror ("Server: cannot open master socket");
		exit (1);
	}

	master.sin_family = AF_INET;
	master.sin_addr.s_addr = INADDR_ANY;
	master.sin_port = htons (strtol(argv[1], NULL, 10));

	if (bind (sock, (struct sockaddr*) &master, sizeof (master))) {
		perror ("Server: cannot bind master socket");
		exit (1);
	}

	listen (sock, 5);


	// The main part
	while(1){	
		fromlength = sizeof (from);
		snew = accept (sock, (struct sockaddr*) & from, & fromlength);
		if (snew < 0) {
			perror ("Server: accept failed");
			exit (1);
		}
		outnum = htonl (number);
		actual_tnum ++;
		if (actual_tnum > tnum){
			tnum *= 2;
			thread = realloc(thread, tnum*sizeof(pthread_t));
		}
		//pthread_create(&thread[actual_tnum], NULL, thread_body, (void *) &id[actual_tnum]);
		thread_body();
	}

	free(whiteboard);
	//fclose(flog);
	
	sleep(1);
	exit(EXIT_SUCCESS);
}
