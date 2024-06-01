CC = gcc
LIBS = -lm -lpng
FLAGS = -O2 -Wall

all: dithering

dithering: dithering.o image.o utils.o
	$(CC) $^ -o dithering $(LIBS)

dithering.o: dithering.c
	$(CC) -c $< $(FLAGS)

image.o: image.c
	$(CC) -c $< $(FLAGS)

utils.o: utils.c
	$(CC) -c $< $(FLAGS)

clean:
	rm -v *.o dithering
