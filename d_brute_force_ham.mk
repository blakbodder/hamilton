
CFLAGS+= -std=c++20

LDFLAGS+= -L/Library/Developer/CommandLineTools/usr/lib   
LDFLAGS+= -L/Library/Developer/CommandLineTools/usr/lib/clang/14.0.0/lib/darwin
#LDFLAGS+= -L/usr/lib -L/usr/local/lib
LDFLAGS+= -lstdc++ -lm -lclang_rt.osx

INCLUDES+= -I/Library/Developer/CommandLineTools/usr/lib/clang/14.0.0/include
#INCLUDES+= -I/Library/Developer/CommandLineTools/usr/include/c++/v1 
#INCLUDES+= -I/usr/include -I/usr/local/include

SOURCES= d_brute_force_ham.cpp
OBJS= d_brute_force_ham.o
BIN= d_brute_force_ham.bin

# all: $(BIN) $(LIB)
all: $(OBJS) $(BIN)

%.o: %.cpp
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wno-deprecated-declarations

%.bin: $(OBJS) 
	$(CC) -o $(basename $@) $(OBJS) $(LDFLAGS) -rdynamic

