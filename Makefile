CC = clang
FLAGS = -Wall -Wextra -O0

all: circuits

clean:
	rm -f circuits

circuits: circuits.c
	${CC} ${FLAGS} circuits.c -o circuits -lm
