# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Header files
HDRS = $(wildcard *.h)

# C files
SRCS = $(wildcard *.c)

# Object files
OBJS = $(SRCS:.c=.o)

# Executable names
EXEC_CLIENT = client_program
EXEC_SERVER = server_program

all: $(EXEC_CLIENT) $(EXEC_SERVER)

client: $(EXEC_CLIENT)

server: $(EXEC_SERVER)

$(EXEC_CLIENT): client.o $(filter-out server.o, $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^

$(EXEC_SERVER): $(filter-out client.o, $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC_CLIENT) $(EXEC_SERVER)
