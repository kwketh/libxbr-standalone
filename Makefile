CC        := g++
CCFLAGS   := -Wall -O3 -fpermissive 
SRC_DIR   := ./
BUILD_DIR := build

PNGFLAGS := $(shell pkg-config libpng --cflags)
PNGLIBS := $(shell pkg-config libpng --libs)

clean:
	@rm -rf $(BUILD_DIR)/*

xb:
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/xbr.c -o $(BUILD_DIR)/xbr.o

hq4x:
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/hq4x.c -o $(BUILD_DIR)/hq4x.o

hq3x:
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/hq3x.c -o $(BUILD_DIR)/hq3x.o

hq2x:
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/hq2x.c -o $(BUILD_DIR)/hq2x.o

cli: hq2x hq3x hq4x
	$(CC) $(CCFLAGS) $(PNGFLAGS) -c $(SRC_DIR)/test_app.c -o $(BUILD_DIR)/cli.o

link: xb hq4x hq3x hq2x cli
	$(CC) $(CCFLAGS) $(PNGFLAGS) $(wildcard build/*.o) -o bin/hq $(PNGLIBS)

all: xb hq4x hq3x hq2x cli link

