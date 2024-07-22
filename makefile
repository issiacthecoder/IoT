BUILDROOT_HOME=/home/issiac/Downloads/buildroot-2024.05/

CC=$(BUILDROOT_HOME)/output/host/usr/bin/arm-linux-gcc

CFLAGS=--sysroot=$(BUILDROOT_HOME)/output/staging
INCLUDES=
LFLAGS=

LIBS=-lcurl -uClibc -lc

SRC=requestor.c
OBJ=$(SRC:.c=.o)
MAIN=test

RM=rm -rf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
 
 
$(MAIN): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJ) $(LFLAGS) $(LIBS)
 	
 
all: $(MAIN)
 
clean:
	rm -f $(MAIN) $(OBJ)



# # CC = arm-linux-gnueabi-gcc
# CC = gcc
# 
# # CCFLAGS =
# 
# INCLUDES =
# 
# LFLAGS = -L/usr/lib/x86_64-linux-gnu
# 
# LIBS = -lcurl -lpthread
# 
# 
# SRC = requestor.c
# 
# OBJ = requestor.o
# 
# MAIN = test
# 
# 
# RM = rm -rf
# 
# 
# %.o: %.c
# 	# $(CC) $(CFLAGS) -c $< -o $@
# 
# 
# $(MAIN): $(OBJ)
# 	# $(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJ) $(LFLAGS) $(LIBS)
# 	
# 
# all: $(MAIN)
# 
# 
# clean:
# 	# rm -f $(MAIN) $(OBJ)

