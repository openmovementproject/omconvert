# make USER_DEFINES="-DNO_MMAP=1"

BIN_NAME = omconvert
CC = gcc
CFLAGS = -std=c99 -O3 -Wall -ffast-math
INC_PATH = -I/usr/local/include
LIB_PATH = -L/usr/local/lib 
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
	$(CC) -o $(BIN_NAME) $(CFLAGS) $(USER_DEFINES) $(SRC) $(INC_PATH) $(LIB_PATH) $(LIBS)

clean:
	rm -f *.o core $(BIN_NAME)


mac_x86: main.c
	$(CC) -o $(BIN_NAME)_x86 $(CFLAGS) $(USER_DEFINES) $(SRC) $(INC_PATH) $(LIB_PATH) $(LIBS) -target x86_64-apple-macos10.12
mac_arm: main.c
	# __arm64__ / __aarch64__
	$(CC) -o $(BIN_NAME)_arm $(CFLAGS) $(USER_DEFINES) $(SRC) $(INC_PATH) $(LIB_PATH) $(LIBS) -target arm64-apple-macos11
mac_universal: mac_x86 mac_arm
	lipo -create -output $(BIN_NAME)_universal $(BIN_NAME)_x86 $(BIN_NAME)_arm


#codesign -vvvv -R="notarized" --check-notarization omconvert_universal

# ---

## Sign executable
#codesign --timestamp --options=runtime -s "Developer ID Application: ***" -v $(BIN_NAME)_universal
## Create a temporary installation for the installler package
#sudo mkdir -p /tmp/omconvert
#cp omconvert_universal /tmp/omconvert/omconvert
## Create installer
#productbuild --identifier "dev.openmovement.omconvert.pkg" --sign "Developer ID Installer: ***" --timestamp --root /tmp/omconvert omconvert.pkg
## Notarize installer
#xcrun altool --notarize-app --primary-bundle-id "dev.openmovement.omconvert.pkg" --username "$EMAIL" --password "$PASSWORD" --file omconvert.pkg
## Wait, use altool XML output
#xcrun altool --notarization-history 0 -u "$EMAIL" -p "$PASSWORD"
## Once notarized, staple ticket
#xcrun stapler staple omconvert.pkg

# ---

# See: https://scriptingosx.com/2021/07/notarize-a-command-line-tool-with-notarytool/
#xcrun notarytool --help
