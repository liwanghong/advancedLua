CC = gcc
TARGET = hello
INCLUDES := ../include

SRC = hello.c

OBJS = $(SRC:%.c=%.o)

$(TARGET): $(OBJS)
	$(CC) -L../lib -llua -o $@ $?

%.o: %.c
	$(CC) -I$(INCLUDES) -c $< -o $@

clean:
	rm $(OBJS) $(TARGET)

