
CC = $(CXX)
CXXFLAGS += -std=c++2a -pedantic -Wall -Wextra

TARGET = spark
SRC = spark.cpp
OBJ = $(SRC:.cpp=.o)


all: $(TARGET)

$(TARGET): $(OBJ)

DEPEND = *.d

%.o: CXXFLAGS += -MMD -MP

-include $(DEPEND)

clean:
	$(RM) $(OBJ) $(DEPEND) $(TARGET)

.PHONY: all clean
