SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
ELF:=client
CC:=gcc
$(ELF):$(OBJS)
	gcc $^ -o $@ -lcrypt -g
clean:
	rm -rf $(OBJS) $(ELF)
