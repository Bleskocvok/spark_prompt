
CC = $(CXX)
CXXFLAGS += -std=c++2a -pedantic -Wall -Wextra

TARGET = prompt
SRC = prompt.cpp
OBJ = $(SRC:.cpp=.o)


all: $(TARGET)

$(TARGET): $(OBJ)

clean:
	$(RM) *.o $(TARGET)

.PHONY: all clean
