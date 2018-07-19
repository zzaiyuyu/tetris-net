#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>

#define DEBUG 1
int first = 1;//判断是否是第一个连接的客户端
int allConnect[1024] = {0};//记录当前所有的客户端套接字
int idx = 0;//上个数组的索引

/****为了反序列化消息，必须知道客户端的模型*****/
#define Weight 10
#define Height 20
struct data {
	int x;
	int y;
};
int background[Height][Weight];
int backColor[Height][Weight];
struct data pos = {.x=3, .y=5};
int shapeNum = 0;//当前图案的序号
int nextBuf = 1;//下一个要出现的图案序号
struct shape {
	int s[5][5];
};
//客户端发送的消息
struct message{
	int types;//消息类型
	struct shape nowShape;
	struct data pos;
	struct shape nextShape;	
	int background[Height][Weight];
	int backColor[Height][Weight];
};

struct message msg;
/********************************/

int startup(int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("socket");
		exit(2);
	}

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(port);

	if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0){
		perror("bind");
		exit(3);
	}

	if(listen(sock, 5) < 0){
		perror("listen");
		exit(4);
	}

	return sock;
}

void SolveCtl(struct message *msg);
void SendPattern(int sock);
void handlerReadyEvents(int epfd, struct epoll_event revs[],\
		int num, int listen_sock)
{
	int i = 0;
	struct epoll_event ev;
	for(; i < num; i++){
		int sock = revs[i].data.fd;
		uint32_t events = revs[i].events;

		if(sock == listen_sock && (events & EPOLLIN)){
			struct sockaddr_in client;
			socklen_t len = sizeof(client);
			int new_sock = accept(sock, (struct sockaddr*)&client, &len);
			if(new_sock < 0){
				perror("accept");
				continue;
			}
			printf("get a new client!\n");
			//关心第一个客户端读事件，和所有连接上的客户的写事件
			if(first){
#ifdef DEBUG 
				printf("第一个客户端上线\n");
#endif
				first = 0;
				ev.events = EPOLLIN;
				ev.data.fd = new_sock;
				epoll_ctl(epfd, EPOLL_CTL_ADD, new_sock, &ev);
				allConnect[idx++] = new_sock;
			}
			else{
#ifdef DEBUG 
				printf("第%d个客户端上线\n", idx+1);
#endif
				allConnect[idx++] = new_sock;
			}
		}
		else if(events & EPOLLIN){
			//第一个客户端的读事件
			ssize_t s = read(sock, &msg, sizeof(msg));
#ifdef DEBUG 
			printf("第一个客户端收到控制信息\n");
#endif
			if(s > 0){
				//处理定时信息和方向控制
				SolveCtl(&msg);
				//该给所有客户端发图案信息了
				int i = 0;
				while(allConnect[i] != 0){
					ev.events = EPOLLOUT;
					ev.data.fd = allConnect[i];
					if(i == 0){
						i++;
						continue;
					}
					else{
						epoll_ctl(epfd, EPOLL_CTL_ADD, allConnect[i], &ev);
					}
					i++;
#ifdef DEBUG 
					printf("设置关心第%d个客户端的写事件\n", i);
#endif
			}	
#ifdef DEBUG 
					printf("第一个客户端的继续读事件\n");
#endif
		}else if(s == 0){
				printf("一号玩家退出游戏，game over\n");
				int i = 0;
				while(allConnect[i]!=0){
					close(allConnect[i]);
				}
				epoll_ctl(epfd, EPOLL_CTL_DEL, allConnect[0], NULL);
			}else{
				perror("read");
			}
		}
		else if(events & EPOLLOUT){
			//写事件就绪的条件，是在第一个客户端读到信息之后	
			//写事件给连接发图案信息	

			SendPattern(sock);
			//其他客户端停止关心写事件
#ifdef DEBUG 
				printf("一个观战客户端的写事件处理完毕，停止关心\n");
#endif
				epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
			
		}else{
			//bug!
		}
	}
}

//处理收到的客户端信息
void SolveCtl(struct message * msg ){
	
#ifdef DEBUG 
	printf("type: %d\n", msg->types);		
	int i, j;
	for(i=0; i<5; i++){
		for(j=0; j<5; j++){
			printf("%d ", msg->nowShape.s[i][j]);
		}
		printf("\n");
	}
#endif 
}
void SendPattern(int sock){
	write(sock ,&msg, sizeof(msg));	

#ifdef DEBUG 
	printf("写入客户端图案信息\n");
#endif
}
int main(int argc, char *argv[])
{
	if(argc != 2){
		printf("Usage: %s [port]\n", argv[0]);
		return 1;
	}

	int listen_sock = startup(atoi(argv[1]));
	int epfd = epoll_create(256);
	if(epfd < 0){
		printf("epoll_create error!\n");
		return 5;
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev);

	struct epoll_event revs[64];
	for(;;){
		int timeout = -1;
		int num = epoll_wait(epfd, revs, \
				sizeof(revs)/sizeof(revs[0]), timeout);
		switch(num){
			case -1:
				printf("epoll_wait error!\n");
				break;
			case 0:
				printf("timeout...\n");
				break;
			default:
				handlerReadyEvents(epfd, revs, num, listen_sock);
				break;
		}
	}
}
