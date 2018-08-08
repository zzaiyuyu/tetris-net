.PHONY:all clean

all:server client viewer

server:server.c
	g++ -o $@ $^
client:game.o client.c
	g++ -g -o $@ $^ -L ./keyboard/keyboard/ -lkeyboard
viewer:viewer.c
	g++ -o $@ $^ -std=gnu99 -L ./keyboard/keyboard/ -lkeyboard


game.o:game.c
	g++   -c game.c -std=gnu99
clean:
	rm *.o
