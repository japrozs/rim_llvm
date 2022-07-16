CC = clang 
CFLAGS = -Wall -Wextra -pedantic -I include -g
LIBFLAGS= -I/usr/local/Cellar/llvm@12/12.0.1_1/include  -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/local/Cellar/llvm@12/12.0.1_1/lib -Wl,-search_paths_first -Wl,-headerpad_max_install_names -lLLVM-12
SRC = $(wildcard src/*.c)
HEAD = $(wildcard include/*.h)
OBJ = $(SRC:.c=.o)
EXEC = out/rimc

all: clean $(OBJ) $(EXEC) $(HEAD)

$(EXEC): $(OBJ)
	$(CC) $(LIBFLAGS) $^ -o $@
	rm -rf src/*.o

%.o: %.cpp
	$(CC) $(CFLAGS) $(LIBFLAGS) $^ -o $@

clean:
	rm -rf *.o src/*.o $(EXEC)
