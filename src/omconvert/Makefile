# make USER_DEFINES="-DNO_MMAP=1"

BIN_NAME = omconvert
CC = gcc
CFLAGS = -O3 -Wall -ffast-math
LIBS = -lm -lpthread

SRC = $(wildcard *.c)
INC = $(wildcard *.h)

COMPILER_VERSION := $(shell $(CC) --version)
ifneq '' '$(findstring clang,$(COMPILER_VERSION))'
  # clang
  #CFLAGS += 
else ifneq '' '$(findstring g++,$(COMPILER_VERSION))'
  # gcc
  CFLAGS += -march=native
else
  # Unknown compiler
  #CFLAGS += 
endif

all: $(BIN_NAME)

$(BIN_NAME): Makefile $(SRC) $(INC)
	$(CC) -std=c99 -o $(BIN_NAME) $(CFLAGS) $(USER_DEFINES) $(SRC) -I/usr/local/include -L/usr/local/lib $(LIBS)

clean:
	rm -f *.o core $(BIN_NAME)


# x86_app: main.c
#     $(CC) main.c -o x86_app -target x86_64-apple-macos10.12
# arm_app: main.c   # __arm64__ / __aarch64__
#     $(CC) main.c -o arm_app -target arm64-apple-macos11
# universal_app: x86_app arm_app
#     lipo -create -output universal_app x86_app arm_app
