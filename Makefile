CC := clang
CFLAGS := -g -Wall -std=c99

TARGET = list

.PHONY: all

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c
