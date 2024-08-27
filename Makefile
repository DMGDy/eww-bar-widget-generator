CC = gcc
CFLAGS = -static -g -Ofast -fno-exceptions -flto -march=native
LDFLAGS = 

TARGET = workspaces
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

PREFIX = ~/Programs/eww/eww/scripts_new
INSTALL_DIR = $(PREFIX)/bin

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
	mkdir -p $(INSTALL_DIR)
	cp $(TARGET) $(INSTALL_DIR)

uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)
