
CXXFLAGS ?= -std=c++20 -pedantic -Wall -Wextra -O2

CPPFLAGS += \
	-Isrc/         \
	-Isrc/grammar  \
	-Isrc/parsing

TARGET = spark

SRC = \
	src/spark.cpp \
	src/grammar/style.cpp \
	src/grammar/segment.cpp \
	src/grammar/color.cpp \
	src/grammar/grammar.cpp

OBJ = $(patsubst src/%.cpp,.obj/%.o,$(SRC))
MAIN_OBJ = .obj/main.o
MAIN_SRC = src/main.cpp

DEPEND = $(OBJ:.o=.d) obj/main.d

TEST = test/test_spark

all: .obj/ $(TARGET)

test: .obj/ test/test_spark
	test/test_spark

$(TEST): test/test_spark.cpp $(OBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

$(TARGET): $(OBJ) $(MAIN_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

.obj/:
	mkdir -p .obj/
	mkdir -p .obj/grammar
	mkdir -p .obj/parsing

.obj/%.o: src/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

.obj/%.o: CXXFLAGS += -MMD -MP


-include $(DEPEND)


clean:
	$(RM) $(OBJ) $(DEPEND)
	$(RM) -r .obj/

distclean: clean
	$(RM) $(TARGET) $(TEST)

.PHONY: all clean distclean prepare test
