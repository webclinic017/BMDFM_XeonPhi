# Makefile

#==============================================================================
# x86-64 Linux, GNU C
#------------------------------------------------------------------------------
CC = g++ -m64
OPTIONS = -Wall -O3 -fPIC
#==============================================================================

MATH = -lm
PSM = -lpthread

all: cflp_udf.o shmem_stub.o simple_comm_buff.o \
     fastlisp BMDFMldr BMDFMsrv CPUPROC

cflp_udf.o: shmem_stub.o simple_comm_buff.o cflp_udf.c cflp_udf.h
	$(CC) $(OPTIONS) -c cflp_udf.c

shmem_stub.o: shmem_stub.c
	$(CC) $(OPTIONS) -c shmem_stub.c

simple_comm_buff.o: cflp_udf.h simple_comm_buff.h simple_comm_buff.cpp
	$(CC) $(OPTIONS) -c simple_comm_buff.cpp

fastlisp: fastlisp.o cflp_udf.o shmem_stub.o simple_comm_buff.o
	$(CC) -o fastlisp fastlisp.o shmem_stub.o cflp_udf.o simple_comm_buff.o \
        $(MATH) $(PSM)

BMDFMldr: BMDFMldr.o cflp_udf.o simple_comm_buff.o
	$(CC) -o BMDFMldr BMDFMldr.o cflp_udf.o simple_comm_buff.o \
        $(MATH) $(PSM)

BMDFMsrv: BMDFMsrv.o cflp_udf.o simple_comm_buff.o
	$(CC) -o BMDFMsrv BMDFMsrv.o cflp_udf.o simple_comm_buff.o \
        $(MATH) $(PSM)

CPUPROC: CPUPROC.o cflp_udf.o simple_comm_buff.o
	$(CC) -o CPUPROC CPUPROC.o cflp_udf.o simple_comm_buff.o \
        $(MATH) $(PSM)

clean:
	rm -f cflp_udf.o shmem_stub.o simple_comm_buff.o \
        fastlisp BMDFMldr BMDFMsrv CPUPROC 2>/dev/null

