  # the compiler gcc for C program, define as g++ for C++
  CC = gcc

  # compiler flags
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  CFLAGS  = -g -Wall
  LIBS = -lpthread -lm
  # the build target executable
  TARGET = Broker stockMarket
  all:

	$(CC) $(CFLAGS) -o broker Broker.c $(LIBS);
	$(CC) $(CFLAGS) -o stockMarket stockMarket.c lib_operaciones.c;
clear:
	rm broker;rm stockMarket;
