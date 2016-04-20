target := yapt

srcs := cameras/perspective io/film io/image structures/triangle_mesh main

type_tests := tests/vec3 tests/vec4 tests/matrix tests/test_pi

tests := tests/transforms

test_dir := tests
src_dir := src
include_dir := $(src_dir)/templates
bin_dir := bin
objs = $(addprefix $(bin_dir)/, $(srcs:=.o))

CXX = g++-5.3.0
INCLUDE_FLAGS := -I"./$(include_dir)" -I"./$(src_dir)" -I"$(GFX)/include"
LD_FLAGS := -L"$(GFX)/lib" -lOpenImageIO $(shell pkg-config --libs OpenEXR)
CXXFLAGS = -std=c++14 -Wall -Wpedantic -Wextra -fcilkplus $(INCLUDE_FLAGS) $(LD_FLAGS)

hpps := $(src_dir)/include/*

MODE ?= release

ifeq ($(MODE), release)
    CXXFLAGS += -DNDEBUG -march=native -O3
else
    CXXFLAGS += -g -O0
endif


.PHONY: all clean

all: $(target)

$(target): $(objs)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(type_tests): %: $(src_dir)/%.cpp
	mkdir -p $(test_dir)
	$(CXX) $(CXXFLAGS) -fopt-info-optimized=$@.opt -o $@ $<

$(tests): %: $(src_dir)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(bin_dir)/%.o: $(src_dir)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -r $(target) $(objs)
