# Default toolchains for Linux builds
CXX = g++
CC = gcc

# Alternative clang toolchains (uncomment if needed)
#CXX = clang++
#CC = clang

# Compiler flags for Linux builds
CXXFLAGS = -Wall -Wextra -std=c++14 -O2
CFLAGS = -Wall -Wextra -std=c11 -O2

# Linked libraries
LIBS = -lncurses

# Binary target and sources
TARGET = plane_game
SRC = main.cpp Game.cpp GameObject.cpp Player.cpp Bullet.cpp Enemy.cpp Display.cpp Boss.cpp
OBJ = $(SRC:.cpp=.o)

# Default target
all: $(TARGET)

# Link rule
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

# Object build rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Cleanup helpers
clean:
	rm -f $(TARGET) $(OBJ)

# Deep cleanup
distclean: clean
	rm -f *~

# Optional install helper
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

.PHONY: all clean distclean install

