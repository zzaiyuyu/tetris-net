#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "./keyboard/keyboard/keyboard.h"

void game(int socket);

int sock;

int main(int argc, char* argv[])
{

	char buf[1024];

	struct sockaddr_in server;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);

	int ret = connect(sock, (struct sockaddr*)&server, sizeof(server));
	if(ret < 0){
		perror("connect");
		return 1;
	}
	printf("connect success\n");

	//game(sock);
}
