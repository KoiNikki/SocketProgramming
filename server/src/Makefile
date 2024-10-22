
CC = gcc  

CFLAGS = -Wall -Wextra -g  

SRCS = auth.c command_parser.c connection_manager.c data_transfer.c error_handling.c file_operations.c main.c  

OBJS = $(SRCS:.c=.o)  

TARGET = server  

all: $(TARGET)  

$(TARGET): $(OBJS)  
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)  

%.o: %.c  
	$(CC) $(CFLAGS) -c $< -o $@  

clean:  
	rm -f $(OBJS) $(TARGET)  
 
.PHONY: all clean