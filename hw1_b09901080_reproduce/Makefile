.PHONY: all clean run
CC=gcc
CFLAGS=-std=c++17 -O2
LDFLAGS=-lstdc++
TARGET=train test
TRAIN_ITER=100

all: $(TARGET)

train: src/train.cpp src/trainer.cpp
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -Iinc

test: src/test.cpp src/Viterbi.cpp
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -Iinc

clean:
	rm -f $(TARGET)

