
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS =
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
EXEC = db

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)