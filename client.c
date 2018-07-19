#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include "./keyboard/keyboard/keyboard.h"
void game();
int sock;

int main(int argc, char* argv[])
{
	if(argc != 3){
		printf("参数不对");
		return 1;
	}

	struct sockaddr_in server;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("socket");
		return 2;
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);
	int ret = connect(sock, (struct sockaddr*)&server, sizeof(server));
	if(ret < 0){
		perror("connect");
		return 3;
	}
	//连接已建立
	//注册服务器信号
	
	printf("connect success\n");
	//游戏主体
	game();

	close(sock);
	return 0 ;
}
