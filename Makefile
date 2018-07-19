.PHONY:all clean

all:server client viewer

server:server.c

client:game.o client.c
	gcc -g -o $@ $^ -L ./keyboard/keyboard/ -lkeyboard
viewer:viewer.c
	gcc -o $@ $^ -std=gnu99 -L ./keyboard/keyboard/ -lkeyboard


game.o:game.c
	gcc   -c game.c -std=gnu99
clean:
	rm *.o
