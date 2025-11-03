SRC_CPP=$(wildcard *.cpp)
SRC_HPP=$(wildcard *.hpp)
MAIN_TARGET=cpplibrary*.cpp
TARGET=$(patsubst %.cpp,%,$(wildcard cpplibrary*.cpp))
all: $(TARGET)
$(TARGET): $(SRC_CPP) $(SRC_HPP)
	g++ -fPIC -shared -o $@.so *.cpp

run:
	python3 main.py
	
.PHONY: clean
clean:
	rm $(TARGET).so 
