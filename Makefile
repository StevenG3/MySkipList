all: skiplist stress
.PHONY: all
CC=g++  
CXXFLAGS=-std=c++0x
CFLAGS=-I
skiplist: main.o 
	$(CC) -o ./bin/main main.o --std=c++11 -pthread 
	rm -f ./*.o

stress: stress_test.o
	$(CC) -o ./bin/stress stress_test.o --std=c++11 -pthread
	rm -f ./*.o

.PHONY: clean
clean:
	rm -f ./*.o
