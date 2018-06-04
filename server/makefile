SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
ELF:=server
CC:=gcc
$(ELF):$(OBJS)
	gcc $^ -o $@ -I /usr/include/mysql/ -lmysqlclient
clean:
	rm -rf $(OBJS) $(ELF)
