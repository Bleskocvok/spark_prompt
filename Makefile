
CXXFLAGS ?= -std=c++17 -pedantic -Wall -Wextra -O2

CPPFLAGS += \
	-I src/         \
	-I src/grammar  \
	-I src/parsing

TARGET = spark
SRC = \
	src/spark.cpp \
	src/parsing/parse.cpp \
	src/grammar/style.cpp \
	src/grammar/segment.cpp \
	src/grammar/color.cpp \
	src/standard.cpp
OBJ = $(patsubst src/%.cpp,obj/%.o,$(SRC))
DEPEND = $(OBJ:.o=.d)


all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^


obj/%.o: src/%.cpp objdir
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

obj/%.o: CXXFLAGS += -MMD -MP


-include $(DEPEND)


objdir:
	mkdir -p obj/
	mkdir -p obj/grammar
	mkdir -p obj/parsing


clean:
	$(RM) $(OBJ) $(DEPEND)

distclean: clean
	$(RM) $(TARGET)

.PHONY: all clean distclean objdir
