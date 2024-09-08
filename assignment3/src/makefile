CXX = g++
CXXFLAGS = -std=c++17 -O3
TARGET = schedular.out
SRC = schedular.cpp

OBJS = $(SRC:.cpp=.o)

all: build

build: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: build
	./$(TARGET) process1.dat

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all build run clean
