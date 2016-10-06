# VARIABLES
# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

# TARGETS
# typing 'make' will invoke the first target entry in the file 
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#

all: ProxyServer

myserver: ProxyServer.c 
	$(CC) $(CFLAGS) -o ProxyServer ProxyServer.c 
	
clean:
	rm *.o 