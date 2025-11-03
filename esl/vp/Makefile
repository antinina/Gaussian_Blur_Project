# SystemC installation directory
SYSTEMC_HOME=/usr/local/systemc-2.3.3

# Compiler to use
CC=g++

# Compiler flags
CFLAGS=-c -I$(SYSTEMC_HOME)/include

# Linker flags
LDFLAGS=-L$(SYSTEMC_HOME)/lib-linux64 -lsystemc

# Source files
SOURCES=main.cpp router.cpp SW.cpp IPB.cpp img_bram.cpp kernel_bram.cpp utils.cpp

# Object files
OBJECTS=$(SOURCES:.cpp=.o)

# Executable name
EXECUTABLE=main

# Default target
all: $(SOURCES) $(EXECUTABLE)

# Build target
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

# Compile target
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
# Run program	
run: 
	./$(EXECUTABLE)	

# Clean target
clean:
	rm -f $(OBJECTS) $(EXECUTABLE) result3x3.txt result25x25.txt






