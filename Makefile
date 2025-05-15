CC = gcc
CFLAGS = -Wall -Wextra -IService -Imodel
LIBS = -lulfius -ljansson -lorcania
SRCS = index.c Service/HelloService.c model/user_model.c
OBJS = $(SRCS:.c=.o)
TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)