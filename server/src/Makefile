
CC = gcc  

CFLAGS = -Wall -g  

SRCS = auth.c command_parser.c connection_manager.c data_transfer.c main.c  

OBJS = $(patsubst %.c,build/%.o,$(SRCS))  

TARGET = server  

all: $(TARGET)  

$(TARGET): $(OBJS)  
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)  

build/%.o: %.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)  
 
.PHONY: all clean