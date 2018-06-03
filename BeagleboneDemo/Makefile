#~ BeaconDemo makefile

# Define files to compile.
SOURCE_DIRECTORY=./src
OBJ= BeaconGTKDemo.o BeaconDriver.o I2C-Wrapper.o
HEADERS = BeaconDriver.h I2C-Wrapper.h
OUTPUT = BeaconDemo

# Compiler option.
CC=gcc
CFLAGS= -std=gnu99 -Wall
PKG = `pkg-config gtk+-2.0 --cflags --libs`

# C compilation rule.
%.o: $(SOURCE_DIRECTORY)/%.c  $(addprefix $(SOURCE_DIRECTORY)/, $(HEADERS))
	@/bin/echo Compiling $(<F)
	@$(CC) -c -o $@ $< $(CFLAGS) $(PKG)

# Main compilation.
all: $(OBJ) $(addprefix $(SOURCE_DIRECTORY)/, $(HEADERS))
	@/bin/echo Creating $(OUTPUT)
	@$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJ) $(PKG)

# Clean up.
clean:
	@/bin/echo Cleaning up...
ifneq ("$(wildcard $(OUTPUT))","")
	@rm $(OUTPUT)
endif
ifneq ("$(wildcard *.o	)","")
	@rm *.o
endif
