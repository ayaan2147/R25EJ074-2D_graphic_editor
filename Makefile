CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -pedantic
LIBS    = -lm
TARGET  = graphics_editor
SRC     = graphics_editor.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	del /f $(TARGET).exe 2>NUL || rm -f $(TARGET)

.PHONY: all clean
