CC = gcc
TARGET = ball.exe
SRC = main.c
OBJ = $(SRC:.c=.o)

CFLAGS = -Wall -Wextra -O2
LDFLAGS = -mwindows
LDLIBS = -lgdi32 -luser32 -lwinmm

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run