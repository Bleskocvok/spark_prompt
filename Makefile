CXXFLAGS ?= -std=c++20 -pedantic -Wall -Wextra -O2

ifdef SANITIZE
	CXXFLAGS += -g -fsanitize=$(SANITIZE) -fsanitize-undefined-trap-on-error
	LDFLAGS += -fsanitize=$(SANITIZE)
endif

INCLUDES = \
	-Isrc/         \
	-Isrc/grammar  \
	-Isrc/style    \
	-Isrc/eval     \
	-Isrc/parsing

CPPFLAGS += $(INCLUDES)

CLANG_TIDY ?= clang-tidy

TARGET = spark

SRC = \
	src/spark.cpp \
	src/bash_renderer.cpp \
	src/style/segment.cpp \
	src/style/color.cpp \
	src/grammar/grammar.cpp

OBJ = $(patsubst src/%.cpp,.obj/%.o,$(SRC))
MAIN_OBJ = .obj/main.o
MAIN_SRC = src/main.cpp

DEPEND = $(OBJ:.o=.d) obj/main.d

TEST = test/test_spark

all: $(TARGET)

test: test/test_spark
	test/test_spark

clang-tidy:
	$(CLANG_TIDY) $(SRC) $(SRC_MAIN) \
	-warnings-as-errors='*' \
	--config-file=.clang-tidy \
	-header-filter=.* -- $(CXXFLAGS) $(INCLUDES)

$(TEST): test/test_spark.cpp $(OBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

$(TARGET): $(OBJ) $(MAIN_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

.obj/created:
	mkdir -p .obj/
	mkdir -p .obj/grammar
	mkdir -p .obj/parsing
	mkdir -p .obj/eval
	mkdir -p .obj/style
	touch .obj/created

.obj/%.o: src/%.cpp .obj/created
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

.obj/%.o: CXXFLAGS += -MMD -MP


-include $(DEPEND)


clean:
	$(RM) $(OBJ) $(DEPEND)
	$(RM) -r .obj/

distclean: clean
	$(RM) $(TARGET) $(TEST)

.PHONY: all clean distclean prepare test clang-tidy
