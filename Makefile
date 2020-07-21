CC=g++
target = go2radio

SRCS=$(wildcard *.cpp)
SRCS+=$(wildcard *.c)
OBJS_cpp= $(patsubst %cpp,%o,$(SRCS))
OBJS= $(patsubst %c,%o,$(OBJS_cpp))

INCLUDE = -I/usr/local/include/
LIB = -L/usr/local/lib/ -lgo2 -L./lib/ugui/

all:$(OBJS)
	$(CC) $(OBJS) -o $(target) $(LIB)

%.o:%.c %.cpp
	$(CC) -c $< -o $@  $(INCLUDE)

clean:
	rm $(OBJS) $(target) -f