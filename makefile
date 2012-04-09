CC=gcc
CFLAGS=-g -Wall -Werror
CFLAGS+=-O1
SRC_FILE=bitmap_steganography.c
OUT_BIN=encoder.exe

all: clean $(OUT_BIN)

$(OUT_BIN):
	$(CC) $(CFLAGS) $(SRC_FILE) -o $(OUT_BIN)

clean:
	-rm $(OUT_BIN)
