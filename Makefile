CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

TARGET = sliding_puzzle
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

rebuild: clean all