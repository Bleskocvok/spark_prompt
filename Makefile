
CXXFLAGS ?= -std=c++20 -pedantic -Wall -Wextra -O2

CPPFLAGS += \
	-I src/         \
	-I src/grammar  \
	-I src/parsing

TARGET = spark

SRC = \
	src/spark.cpp \
	src/grammar/style.cpp \
	src/grammar/segment.cpp \
	src/grammar/color.cpp

OBJ = $(patsubst src/%.cpp,.obj/%.o,$(SRC))
DEPEND = $(OBJ:.o=.d)


all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

.obj/:
	mkdir -p .obj/
	mkdir -p .obj/grammar
	mkdir -p .obj/parsing

.obj/%.o: src/%.cpp .obj/
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

.obj/%.o: CXXFLAGS += -MMD -MP


-include $(DEPEND)


clean:
	$(RM) $(OBJ) $(DEPEND)
	$(RM) -r .obj/

distclean: clean
	$(RM) $(TARGET)

.PHONY: all clean distclean prepare
