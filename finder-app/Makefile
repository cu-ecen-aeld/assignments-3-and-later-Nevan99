CC = $(CROSS_COMPILE)gcc

CFLAGS = -Wall -Wextra -Werror -g

TARGET = writer

SOURCES = writer.c

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)


$(TARGET): $(OBJECTS) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
.PHONY: all clean