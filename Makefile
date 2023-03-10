include Makefile.vars

all: skiplist stress
.PHONY: all
CC=g++  
CXXFLAGS=-std=c++0x
CFLAGS=-I

MAKEF=$(MAKE) -f Makefile.in

prepare:
	$(MKDIR) bin

skiplist: main.o 
	$(CC) -o ./bin/main main.o --std=c++11 -pthread 
	rm -f ./*.o

stress: stress_test.o
	$(CC) -o ./bin/stress stress_test.o --std=c++11 -pthread
	rm -f ./*.o

protorpc: protorpc_client protorpc_server

protorpc_protoc:
	bash rpc/protorpc/proto/protoc.sh

protorpc_client: prepare protorpc_protoc
	$(MAKEF) TARGET=$@ SRCDIRS="$(CORE_SRCDIRS) cpputil include/evpp rpc/protorpc/generated" \
		SRCS="rpc/protorpc/protorpc_client.cpp rpc/protorpc/protorpc.c" \
		LIBS="protobuf"

protorpc_server: prepare protorpc_protoc
	$(RM) rpc/protorpc/*.o
	$(MAKEF) TARGET=$@ SRCDIRS="$(CORE_SRCDIRS) cpputil include/evpp rpc/protorpc/generated" \
		SRCS="rpc/protorpc/protorpc_server.cpp rpc/protorpc/protorpc.c" \
		LIBS="protobuf"


.PHONY: clean
clean:
	rm -f ./*.o
