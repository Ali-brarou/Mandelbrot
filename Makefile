TARGET = fractal
CC = gcc
CFLAGS = -ftree-vectorize -funroll-loops -ffast-math -march=native -DNDEBUG 

all : 
	$(CC) main.c -O3 -Wall -Wextra -I../libs/stb -lm -fopenmp $(CFLAGS) -o $(TARGET)
