/*
	C socket server example
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write
//#include <pthread>

#define MAX_LEN 128
//global vars
// when main function called, changes by the main's parameters
int _debug_mode = 0;
int _client_port = 6000;
int _peer_port = 6001;
int _max_clients = 10;
char _peers[MAX_LEN] = "peers.dat";
char _storage[MAX_LEN] = "data.dat";

pthread_mutex_t mutex;
pthread_t  thread;
pthread_attr_t attr;

char ids[MAX_LEN][MAX_LEN];
char contents[MAX_LEN][MAX_LEN];
int count = 0;
int f_bye = 0;

//input parameter parsing
int parse_arg(int nparam, char *param[]) {
	int i;
	int p = 0, P = 0, t = 0, s = 0, storage = 0;
	char *tmp;

	for (i = 1; i < nparam; i++) {
		if (strcmp(param[i], "-d") == 0) {
			_debug_mode = 1; 
			continue;
		}		
		else if (strcmp(param[i], "-p") == 0) {
			p = 1;
			continue;
		}
		else if (strcmp(param[i], "-P") == 0) {
			P = 1;
			continue;
		}
		else if (strcmp(param[i], "-t") == 0) {
			t = 1;
			continue;
		}
		else if (strcmp(param[i], "-s") == 0) {
			s = 1;
			continue;
		}
		

		if (p) {
			_client_port = atoi(param[i]);
			p = 0;
		}
		else if (P) {
			_peer_port = atoi(param[i]);
			P = 0;
		}
		else if (t) {
			_max_clients = atoi(param[i]);
			t = 0;
		}
		else if (s) {
			//_peers = param[i];
			strcpy(_peers, param[i]);
			s = 0;
		}
		else {
			//_storage = param[i];
			strcpy(_storage, param[i]);
		}

	}
	return 1;

}

int find_id(char *id) {
	int i = 0;
	while (1) {
		if (strcmp(id, ids[i]) == 0)return i;
		if (ids[i] == NULL) return -1;
		if (i >= MAX_LEN) return -1;
		i++;
	}	
	return -1;
}
//socket server fucntion
void* server_thread(void *Param) {
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[MAX_LEN], str_ret[MAX_LEN], str_cont[MAX_LEN];
	char *pmethod, *pid, *pcontent;	


    	int result, id, ind;
    


	int pipefd[2];
	result = pipe(pipefd);

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( _peer_port );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) != 0)
	{
		//print the error message
		printf("----jch----%s/%d\n", __FILE__, __LINE__);
		f_bye = 1;
		return NULL;
	}
	puts("bind done");
	
	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	
	//accept connection from an incoming client
	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (client_sock < 0)
	{
		perror("accept failed");
		f_bye = 1;
		return NULL;
	}
	puts("Connection accepted");
	
	//Receive a message from client
	while( (read_size = recv(client_sock , client_message , MAX_LEN , 0)) != 0 )
	{	
		printf ("---THREAD LOOP---\n");

		pmethod = strtok(client_message, " ");

		if (pmethod == NULL) {
			strcpy(client_message, "NO METHOD FOUND");	
			write(client_sock , client_message , strlen(client_message));
			memset (client_message, 0x00, MAX_LEN);
			continue;
		}
		if (strcmp(pmethod, "READ") == 0) {
			pid = strtok(client_message + strlen(pmethod) + 1, " ");
			if (pid == NULL) {
				strcpy(client_message, "NO ID FOUND");	
				write(client_sock , client_message , strlen(client_message));	
				memset (client_message, 0x00, MAX_LEN);
				continue;
			}
			ind = find_id(pid);
			if (ind == -1) strcpy(client_message, "NO ID CONTENT FOUND");
			else strcpy(client_message, contents[ind]);
		}
		else if (strcmp(pmethod, "WRITE") == 0) {
			pid = strtok(client_message + strlen(pmethod) + 1, " ");
			if (pid == NULL) {
				strcpy(client_message, "NO ID FOUND");
				write(client_sock , client_message , strlen(client_message));	
				memset (client_message, 0x00, MAX_LEN);
				printf ("---WRITE METHOD pid null---\n");
				continue;
			}
			pcontent = strtok(client_message + strlen(pmethod) + strlen(pid) + 2, " ");
			if (pcontent == NULL) {
				strcpy(client_message, "NO CONTENT FOUND");
				write(client_sock , client_message , strlen(client_message));	
				memset (client_message, 0x00, MAX_LEN);
				printf ("---WRITE METHOD pcontent null---\n");
				continue;
			}
			ind = find_id(pid);
			if (ind != -1) strcpy(client_message, "ID EXIST");
			else {
				strcpy(ids[count], pid);
				strcpy(contents[count], pcontent);
				count++;
				strcpy(client_message, pcontent);
			}

		}
		else if (strcmp(pmethod, "REPLACE") == 0) {
			pid = strtok(client_message + strlen(pmethod) + 1, " ");
			if (pid == NULL) {
				strcpy(client_message, "NO ID FOUND");	
				write(client_sock , client_message , strlen(client_message));	
				memset (client_message, 0x00, MAX_LEN);
				continue;
			}
			pcontent = strtok(client_message + strlen(pmethod) + strlen(pid) + 2, " ");
			if (pcontent == NULL) {
				strcpy(client_message, "NO CONTENT FOUND");	
				write(client_sock , client_message , strlen(client_message));	
				memset (client_message, 0x00, MAX_LEN);
				continue;
			}
			ind = find_id(pid);
			if (ind == -1) strcpy(client_message, "ID NOT EXIST");
			else {
				strcpy(contents[ind], pcontent);
				strcpy(client_message, pcontent);
			}

		}
		else if (strcmp(pmethod, "BYE") == 0) {
			puts("---program ends---");	
				
			f_bye = 1;
		}
		else {
			strcpy(client_message, "UNKNOWN METHOD");
		}

		printf ("--send to client : %s\n", client_message);

		//Send the message back to client
		write(client_sock , client_message , strlen(client_message));
		memset (client_message, 0x00, MAX_LEN);
	}
	f_bye = 1;
}
int main(int argc , char *argv[])
{
	int i;
	FILE *pf;
	char line[MAX_LEN];
	char *ps;
	
	parse_arg(argc, argv);
//	printf ("-----  -2 -----\n");
	printf ("debug mode : %d\n", _debug_mode);
	printf ("client port : %d\n", _client_port);
	printf ("peer port : %d\n", _peer_port);
	printf ("max clients  %d\n", _max_clients);
	printf ("peer file : %s\n", _peers);
	printf ("storage file : %s\n", _storage);
//	printf("----  -1-----\n");

	pf = freopen(_storage, "r", stdin);
//	printf("----0-----");
	if (pf == NULL) {
		if (_debug_mode) printf ("storage file not found");
		return 0;
	}
	i = 0;
//	printf("----1-----");
	while (fgets(line, MAX_LEN, pf)) {
		ps = strtok(line, " ");
		strcpy(ids[i], ps);
		strcpy(contents[i], line + strlen(ps) + 1);
		i++;
	}
//	printf("----2-----");
	count = i;
	
	fclose(pf);

	//pthread_mutex_init(&mutex, &attr);
	
	for (i = 0; i < 1; i++) {	
		pthread_create(&thread, &attr, server_thread, NULL);
	}
	
	while(1) {
		if (f_bye) break;
	}
	//pthread_mutex_destroy(&mutex);

	pf = freopen(_storage, "w", stdout);
	if (pf == NULL) {
		if (_debug_mode) printf ("storage file not found");
		return 0;
	}

//	printf ("----count %d-----\n", count);
	for (i = 0; i < count; i++) {
		if (ids[i] == NULL) {
			break;
		}
		sprintf (line, "%s %s\n", ids[i], contents[i]);
		fputs(line, pf);
		
	}
	fclose(pf);

	return 0;
}
