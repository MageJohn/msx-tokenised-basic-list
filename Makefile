CC := clang
CCFLAGS := -g -Wall

TARGET = list

.PHONY: all

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c