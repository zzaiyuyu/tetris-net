#include <termios.h>
#include <unistd.h>
#include <stdio.h>

//系统，库，自己
int main()
{

	int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
	struct itimespec ts;
		
	ts.it_value.tv_nesc = 1;
	ts.it_interval.
	
	int c;
	struct termios ts;
	tcgetattr(0, &ts);
	ts.c_lflag &= ~ICANON;
	ts.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW, &ts);

	while((c=getchar())!=EOF){
		putchar(c);
	}

	ts.c_lflag |= ICANON;
	ts.c_lflag |= ECHO;
	tcsetattr(0, TCSANOW, &ts);
}
