SRC_CPP=$(wildcard *.cpp)
SRC_HPP=$(wildcard *.hpp utils.hpp)
MAIN_TARGET=main*.cpp
TARGET=$(patsubst %.cpp,%,$(wildcard main*.cpp))
all: $(TARGET)
$(TARGET): $(SRC_CPP) $(SRC_HPP)
	g++ -o $@ *.cpp

run:
	./$(TARGET)	
	
.PHONY: clean
clean:
	rm $(TARGET) gaussian_blur_results*.txt
