CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lssl -lcrypto -lpthread
TARGET = executeables/server
SRC = code/server.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -O3 $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
