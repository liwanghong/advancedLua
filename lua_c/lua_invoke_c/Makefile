CC = gcc
TARGET = hello.so
INCLUDES := ../include

SRC = hello.c

OBJS = $(SRC:%.c=%.o)

$(TARGET): $(OBJS)
	$(CC) -L../lib -llua -shared -o $@ $?

%.o: %.c
	$(CC) -I$(INCLUDES) -c $< -o $@

clean:
	rm $(OBJS) $(TARGET)

