
CFLAGS+= -std=c++20

LDFLAGS+= -L/Library/Developer/CommandLineTools/usr/lib   
LDFLAGS+= -L/Library/Developer/CommandLineTools/usr/lib/clang/14.0.0/lib/darwin
LDFLAGS+= -L/usr/local/lib
#LDFLAGS+= -L/usr/lib
LDFLAGS+= -lOsiClp -lClpSolver -lCoinUtils -lCbc -lCbcSolver -lbz2 -lz
LDFLAGS+= -lstdc++ -lm -lclang_rt.osx

INCLUDES+= -I/Library/Developer/CommandLineTools/usr/lib/clang/14.0.0/include
#INCLUDES+= -I/Library/Developer/CommandLineTools/usr/include/c++/v1 
#INCLUDES+= -I/usr/include -I/usr/local/include
INCLUDES+= -I/usr/local/include/coin

SOURCES= d_cbc_hamiltonian.cpp
OBJS= d_cbc_hamiltonian.o
BIN= d_cbc_hamiltonian.bin

# all: $(BIN) $(LIB)
all: $(OBJS) $(BIN)

%.o: %.cpp
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wno-deprecated-declarations

%.bin: $(OBJS) 
	$(CC) -o $(basename $@) $(OBJS) $(LDFLAGS) -rdynamic

