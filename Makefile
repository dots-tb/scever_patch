CC=gcc
CFLAGS=
LDFLAGS=
SOURCES=main.c
EXECUTABLE=scever_patch
all:
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(EXECUTABLE)
clean:
	rm -rf $(EXECUTABLE)
