#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include "llist.h"
#include "bitmap.h"
#define DEBUG 1

List conList;
Bitmap bmp;
int playToRoom[10240] = {0};
List* playerQue[10240] = {0};
int idx = 1;

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
	int room;
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

void SolveCtl(struct message*  msg);
int SolvePlayer(int epfd, int sock, int room){
	if(bmp.exist(room)){
		return -1;
	}
	bmp.set(room);
	List* pl = new List;
	playerQue[room] = &(*pl);
	playToRoom[sock] = room;
	printf("room%d被玩家%d创建\n", room, sock);
	
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);
	printf("设置关心room%d的%d玩家读事件\n", room, sock);
	return 1;
}

int SolveViewer(int epfd, int sock, int room){
	if(!bmp.exist(room)){
		return -1;
	}
	List* pl = playerQue[room];
	pl->push(sock);	
	playToRoom[sock] = room;
	printf("room%d加入新的观战者%d\n", room, sock);
	//此时文件描述符还是读事件监听，需要删掉
	struct epoll_event ev;
	epoll_ctl(epfd, EPOLL_CTL_DEL,sock, &ev);
	return 1;
}
void SolveData(int epfd, int room){
	struct epoll_event ev;
	Node* pCur= playerQue[room]->head();
	pCur = pCur->next;
	while(pCur){
		ev.events = EPOLLOUT;
		ev.data.fd = pCur->sock;
		epoll_ctl(epfd, EPOLL_CTL_ADD,pCur->sock, &ev);
		printf("设置关心room%d的%d观战者的写事件\n", room, pCur->sock);
		pCur = pCur->next;
	}
}

void SendPattern(int epfd, int sock);
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
			ev.events = EPOLLIN;
			ev.data.fd = new_sock;
			epoll_ctl(epfd, EPOLL_CTL_ADD, new_sock, &ev);
			conList.push(new_sock);
			printf("一个新客户端登录\n");
		}
		else if(events & EPOLLIN){
			ssize_t s = read(sock, &msg, sizeof(msg));
			if(s > 0){
				SolveCtl(&msg);
				//1 玩家 2 viewer 3 玩家的数据
				switch(msg.types){
					case 1:
						if(-1 == SolvePlayer(epfd, sock, msg.room)){
							printf("请求房间号已存在\n");
							close(sock);			
						}
						break;
					case 2:
						if(-1 == SolveViewer(epfd, sock, msg.room)){
							printf("没有这个直播间\n");
							close(sock);
						}
						break;
					case 3:
						SolveData(epfd, msg.room);
						break;
					default:
						break;
				}
			}else if(s == 0){
				printf("%d号玩家退出游戏，room%d关闭\n", sock, playToRoom[sock]);
				int room = playToRoom[sock];
				List* pView = playerQue[room];
				Node* pCur = playerQue[room]->head()->next;
				while(pCur){
					printf("停止关心room%d的%d观战者的写事件\n", room, pCur->sock);
					epoll_ctl(epfd, EPOLL_CTL_DEL, pCur->sock, &ev);
					close(pCur->sock);
					Node* pDel = pCur;
					pCur = pCur->next;
					pView->del(pDel->sock);
				}			
				bmp.reset(playToRoom[sock]);
				delete pView;
				close(sock);
			}
			else{
				perror("read");
			}
		}
		else if(events & EPOLLOUT){
			//写事件给连接发图案信息	
			SendPattern(epfd, sock);
			printf("%d观战客户端的写事件处理完毕，停止关心\n", sock);
			epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
		}else{
			//bug!
		}
	}
}

//处理收到的客户端信息
void SolveCtl(struct message* msg ){
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
void SendPattern(int epfd, int sock){
	int ret = write(sock ,&msg, sizeof(msg));	
	if(errno == EPIPE){
		printf("写入客户端图案信息失败");
		int room = playToRoom[sock];
		List* pView = playerQue[room];
		printf("关闭room%d, sock%d\n",room,  sock);
		pView->del(sock);
		epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
		close(sock);
		return;
	}
	printf("写入客户端图案信息\n");
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

	signal(SIGPIPE, SIG_IGN);

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev);

	//初始化连接链表
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
