AR = ar
CC = gcc

CFLAGS = -g -O0 -w\
		 -L/usr/lib \
		 -I/usr/include/ \
		 -I./
COMPILE = $(CC) $(CFLAGS) -c
LINK = $(CC) $(CFLAGS)
SRCS := fs_config.c 
OBJS := $(addprefix $(OBJ_PATH)/, $(patsubst %.c, %.o,$(SRCS)))

all:fs_config distclean
	
fs_config:./fs_config.o $(LIBA)
	$(LINK) $< -o $@ 
	
fs_config.o:fs_config.c
	$(COMPILE) $< -o $@
	

.PHONY:clean distclean
clean:
	rm -rf fs_config 
distclean:
	rm -rf fs_config.o 
	
