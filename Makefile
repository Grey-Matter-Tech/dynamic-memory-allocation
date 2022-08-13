CC=gcc
CFLAGS=-I.
DEPS = dma.h, utils.h
OBJ = sample_application.o dma.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

dmaMake: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)