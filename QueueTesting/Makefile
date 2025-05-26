CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lssl -lcrypto -lpthread
TARGET_DIR = ../servers
SOURCES = array.c heap.c linked.c none.c
TARGETS = $(SOURCES:.c=) # This replaces .c with empty string for targets

all: $(TARGETS)

# Ensure TARGET_DIR exists
$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

# Compile each source file to an executable in TARGET_DIR
$(TARGETS): %: %.c | $(TARGET_DIR)
	$(CC) $(CFLAGS) $< -o $(TARGET_DIR)/$@ $(LDFLAGS)

clean:
	rm -f $(TARGET_DIR)/*
