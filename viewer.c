#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "./keyboard/keyboard/keyboard.h"

//前景色背景色，宽高，左上边界
int FC =  5;
#define BC 7
#define Weight 10
#define Height 20
#define MAR_LEFT 30
#define MAR_TOP	10
struct data {
	int x;
	int y;
};
//用来判断背景数组是否有图案
int background[Height][Weight];
//用来保存背景的每种图形的颜色
int backColor[Height][Weight];

//贯穿整个游戏的位置信息,当前图形的左上角点
struct data pos = {.x=3, .y=5};
int shapeNum = 0;//当前图案的序号
int nextBuf = 1;//下一个要出现的图案序号

struct shape {
	int s[5][5];
};

//在client里定义过了
extern int  sock;
//给服务器发送的消息
struct message{
	int types;//消息类型
	struct shape nowShape;
	struct data pos;
	struct shape nextShape;	
	int background[Height][Weight];
	int backColor[Height][Weight];
};

void SendMsg();
//用5x5数组模拟图案
struct shape shape_arr[7] = {
	{ 0,0,0,0,0, 0,0,1,0,0, 0,1,1,1,0, 0,0,0,0,0, 0,0,0,0,0},
	{ 0,0,0,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,0,0,0},
	{ 0,0,0,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,1,0, 0,0,0,0,0},
	{ 0,0,0,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,1,1,0,0, 0,0,0,0,0},
	{ 0,0,0,0,0, 0,1,1,0,0, 0,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0},
	{ 0,0,0,0,0, 0,1,1,0,0, 0,0,1,1,0, 0,0,0,0,0, 0,0,0,0,0},
	{ 0,0,0,0,0, 0,0,1,1,0, 0,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0} 
};

//利用VT100终端控制
//在坐标位置打印一个块
void draw_element(int x, int y, int c)
{
	//设置光标位置
	//设置前景，背景色
	//画[]
	x *= 2;
	x += MAR_LEFT;
	y += MAR_TOP;
	printf("\033[%d;%dH", y, x);
	printf("\033[3%dm\033[4%dm", c, c);
	printf("[]");
	//消去光标
	printf("\033[?25l");
	printf("\033[0m");
	fflush(stdout);
}

//画一个完整的图形,x是列坐标
void draw_shape(int x, int y, struct shape p, int c)
{
	//打印5x5数组
	for (int i=0; i<5; i++) {
		for (int j=0; j<5; j++) {
			if (p.s[i][j] != 0 )
				draw_element(x+j, y+i, c);
		}

	}
}
//画整个游戏界面
void draw_bk( void )
{
	//主游戏界面
	for (int i=0; i<Height; i++) {
		for (int j=0; j<Weight; j++) {
			if ( background[i][j] == 0 )
				draw_element(j, i, BC);
			else
				draw_element(j, i, backColor[i][j]);
		}
	}
	//下一个图案区域
	for(int i=0; i<5; i++){
		for(int j=0; j<5; j++){
			draw_element(10+j, 5+i, BC);
		}
	}
	draw_shape(10, 5, shape_arr[nextBuf], FC);
}

void ShowMsg(struct message * msg ){
	
	int i, j;
	for(i=0; i<Height; i++){
		for(j=0; j<Weight; j++){
			backColor[i][j] = msg->backColor[i][j];
			background[i][j] = msg->background[i][j];	
			//printf("%d ", background[i][j]);
		}
		//printf("\n");
	}
	//实时刷新
	draw_bk();
	draw_shape(msg->pos.x, msg->pos.y, msg->nowShape ,FC);
}

void handler_int()
{
	recover_keyboard();
	//显示光标，恢复位置，清屏
	printf("\033[?25h");
	printf("\033[u");
	printf("\033[2J");
	exit(0);
}
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
	printf("connect success\n");

	signal(SIGINT, handler_int);

	printf("\033[2J");
	init_keyboard();
	while(1){
		struct message msg;
		int readSize = read(sock, &msg, sizeof(msg));	
		if(readSize <0){
			perror("read");
			continue;
		}
		if(readSize == 0){
			printf("server close\n");
			handler_int();
			close(sock);
			exit(4);
		}
		ShowMsg(&msg);
	}
	return 0 ;
}
