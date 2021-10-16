
CC = $(CXX)
CXXFLAGS += -std=c++2a -pedantic -Wall -Wextra

TARGET = spark
SRC = spark.cpp
OBJ = $(SRC:.cpp=.o)


all: $(TARGET)

$(TARGET): $(OBJ)

clean:
	$(RM) *.o $(TARGET)

.PHONY: all clean
