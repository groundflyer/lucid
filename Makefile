target := yapt

srcs := main

type_tests :=	tests/vector \
				tests/matrix \
				tests/test_pi \
				tests/normal \
				tests/transforms \
				tests/rng \
				tests/ray \
				tests/literal \
				tests/sphere \
				tests/aabb \
				tests/plane \
				tests/disk \
				tests/triangle \
				tests/generic_primitive \
				tests/perspective_camera \
				tests/struct_binding

tests := tests/image

test_dir := tests
src_dir := src
bin_dir := bin
objs = $(addprefix $(bin_dir)/, $(srcs:=.o))

CXX := g++-7.3.0
INCLUDE_FLAGS := -I"./$(src_dir)" -I"./tinytimer"
CXXFLAGS = -std=c++17 -Wall -Wpedantic -Wextra -pipe

hpps := $(src_dir)/core/*.hpp

MODE ?= release

ifeq ($(MODE), release)
    CXXFLAGS += -DNDEBUG -march=native -O3 -ffast-math
else
    CXXFLAGS += -g -O0 -D_GLIBCXX_DEBUG
endif

ifeq ($(CXX), clang++)
    CXXFLAGS += -stdlib=libc++ -Rpass=loop-vectorize -fcolor-diagnostics
else
    CXXFLAGS += -fopt-info-vec
endif


.PHONY: all clean

all: $(target)

$(target): $(objs)
	$(CXX) $(CXXFLAGS) $(LD_FLAGS) $(INCLUDE_FLAGS) -o $@ $^

$(type_tests): %: $(src_dir)/%.cpp $(hpps)
	mkdir -p $(test_dir)
	$(CXX) $(CXXFLAGS) $(INCLUDE_FLAGS) -o $@ $<

$(tests): %: $(src_dir)/%.cpp $(hpps)
	mkdir -p $(test_dir)
	$(CXX) $(CXXFLAGS) $(INCLUDE_FLAGS) $(LD_FLAGS) -o $@ $<

$(bin_dir)/%.o: $(src_dir)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -r $(target) bin/ tests/
