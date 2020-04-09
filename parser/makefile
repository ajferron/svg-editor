CFLAGS = -Wall -g -std=c11
BIN = bin/
OBJ = obj/
INC = include/
SRC = src/
XML_PATH = /usr/include/libxml2

# export LD_LIBRARY_PATH=./bin:.

# Standard common makefile
parser: libsvgparse.so

libsvgparse.so: $(OBJ)SVGParser.o $(OBJ)LinkedListAPI.o
	gcc -shared $(OBJ)SVGParser.o $(OBJ)LinkedListAPI.o -lxml2 -lm -o ../libsvgparse.so

$(BIN)libsvgparse.so: $(OBJ)SVGParser.o $(OBJ)LinkedListAPI.o
	gcc -shared $(OBJ)SVGParser.o $(OBJ)LinkedListAPI.o -lxml2 -lm -o $(BIN)libsvgparse.so

$(OBJ)SVGParser.o: $(OBJ) $(SRC)SVGParser.c $(INC)LinkedListAPI.h $(INC)SVGParser.h
	gcc -c -fpic $(CFLAGS) -I$(XML_PATH) -I$(INC) $(SRC)SVGParser.c -o $(OBJ)SVGParser.o

$(OBJ)LinkedListAPI.o: $(OBJ) $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	gcc -c -fpic $(CFLAGS) -I$(INC) $(SRC)LinkedListAPI.c -o $(OBJ)LinkedListAPI.o

$(OBJ):
	mkdir $(OBJ)

main: main.c parser $(BIN)libsvgparse.so
	gcc $(CFLAGS) -I$(INC) -I$(XML_PATH) -Lbin -lsvgparse main.c -o bin/main

run: main
	./bin/main

clean:
	rm -f $(BIN)*.so $(BIN)main
	rm -rf $(OBJ)
