
CC = $(CXX)
CXXFLAGS ?= -std=c++17 -pedantic -Wall -Wextra -O2

TARGET = spark
SRC = src/spark.cpp
OBJ = $(patsubst src/%.cpp,%.o,$(SRC))
DEPEND = $(OBJ:.o=.d)


all: $(TARGET)

$(TARGET): $(OBJ)

%.o: src/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


%.o: CXXFLAGS += -MMD -MP

-include $(DEPEND)


clean:
	$(RM) $(OBJ) $(DEPEND)

distclean: clean
	$(RM) $(TARGET)

.PHONY: all clean distclean
