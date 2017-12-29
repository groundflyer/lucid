target := yapt

srcs := cameras/perspective \
	io/film \
	io/image \
	structures/triangle_mesh \
	scene/geometry_object \
	scene/material \
	scene/omni_light \
	scene/traverser \
	scene/scene \
	main

type_tests :=	tests/vec3 \
		tests/vec4 \
		tests/matrix \
		tests/test_pi \
		tests/normal \
		tests/transforms \
		tests/rng

test_dir := tests
src_dir := src
include_dir := $(src_dir)/templates
bin_dir := bin
objs = $(addprefix $(bin_dir)/, $(srcs:=.o))

CXX := g++-7.2.0
INCLUDE_FLAGS := -I"./$(include_dir)" -I"./$(src_dir)"
LD_FLAGS := -lOpenImageIO $(shell pkg-config --libs OpenEXR)
CXXFLAGS = -std=c++17 -Wall -Wpedantic -Wextra -pipe

hpps := $(src_dir)/include/*

MODE ?= release

ifeq ($(MODE), release)
    CXXFLAGS += -DNDEBUG -march=native -O3
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

$(type_tests): %: $(src_dir)/%.cpp
	mkdir -p $(test_dir)
	$(CXX) $(CXXFLAGS) $(INCLUDE_FLAGS) -o $@ $<


$(bin_dir)/%.o: $(src_dir)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -r $(target) bin/ tests/
