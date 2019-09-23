/*
	C ECHO client example using sockets
*/
#include <stdio.h>	//printf
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr

#define MAX_LEN 128
char _peer_ip[MAX_LEN] = "192.168.2.121";
int  _peer_port = 6001;
int _debug_mode = 0;


int parse_arg(int nparam, char *param[]) {
	int i;
	int a = 0, P = 0, p = 0;
	char *tmp;

	printf("--%d-%s, %s---", nparam, param[1], param[2]);
	for (i = 1; i < nparam; i++) {
		if (strcmp(param[i], "-d") == 0) {
			_debug_mode = 1; 
			continue;
		}
		else if (strcmp(param[i], "-a") == 0) {
			a = 1; 
			continue;
		}		
		
		else if (strcmp(param[i], "-P") == 0) {
			P = 1;
			continue;
		}

		if (P) {
			_peer_port = atoi(param[i]);
			P = 0;
		}
		else if (a) {
			//_peer_ip = param[i];
			strcpy(_peer_ip, param[i]);
			a = 0;
		}

	}
	/*if (nparam > 1) {
		for (i = 2; i < 4; i++) {
			if (i == 2 && param[i]) strcpy(_peer_ip, param[i]);
			if (i == 3 && param[i]) _peer_port = atoi(param[i]);
		}
	}*/
	return 1;

}


void client_loop() {
	int sock;
	struct sockaddr_in server;
	char message[MAX_LEN] , server_reply[MAX_LEN];
	
	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");
	
	server.sin_addr.s_addr = inet_addr(_peer_ip);
	server.sin_family = AF_INET;
	server.sin_port = htons( _peer_port );
	
//	int on = 1;
//	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}
	
	puts("Connected\n");
	
	//keep communicating with server
	while(1)
	{
		printf("Enter message : ");
		gets(message);
		printf ("%s", message);
		//Send some data
		if( send(sock , message , strlen(message) , 0) < 0)
		{
			puts("Send failed");
			return 1;
		}
		memset(server_reply, 0x00, MAX_LEN);
		recv(sock , server_reply , MAX_LEN , 0);
		printf("++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		puts(server_reply);
	}
	
	close(sock);
	return 0;
}

int main(int argc , char *argv[])
{
	int i;
//	printf("%d\n", argc);
//	for (i = 1; i <= argc; i++) printf("%s\n", argv[i]);
	
	parse_arg(argc, argv);
	printf ("debug mode : %d\n", _debug_mode);
//	printf ("client port : %d\n", _client_port);
	printf ("peer ip : %s\n", _peer_ip);
	printf ("peer port : %d\n", _peer_port);

	client_loop();
	
	return 0;	
}
