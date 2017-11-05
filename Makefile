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

hq4x: xb
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/hq4x.c -o $(BUILD_DIR)/hq4x.o

hq3x: xb
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/hq3x.c -o $(BUILD_DIR)/hq3x.o

hq2x: xb
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/hq2x.c -o $(BUILD_DIR)/hq2x.o

fixpng:
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/fixpng.c -o $(BUILD_DIR)/fixpng.o

cli: fixpng hq2x hq3x hq4x
	$(CC) $(CCFLAGS) $(PNGFLAGS) -c $(SRC_DIR)/test_app.c -o $(BUILD_DIR)/hqcli.o

link: cli
	$(CC) $(CCFLAGS) $(PNGFLAGS) $(wildcard build/xbr.o build/hq*.o) -o build/hq $(PNGLIBS)
	$(CC) $(CCFLAGS) $(PNGFLAGS) build/fixpng.o -o build/fixpng $(PNGLIBS)

all: xb hq4x hq3x hq2x cli link

