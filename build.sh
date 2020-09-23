#!/bin/bash

showhelp() {
    echo "Usage: $0 [-t] [-b] [-e] [-a]"
    echo -e "\t-t\tbuild and run tests"
    echo -e "\t-b\tbuild and run benchmarks"
    echo -e "\t-e\tuse temporary directory"
    echo -e "\t-a\tbuild benchmarks and tests then run them"
}

SRC_DIR=`realpath $(dirname "${BASH_SOURCE[0]}")`
BUILD_DIR=${SRC_DIR}/build_clang
CMAKE_ARGS=(
    -DCMAKE_CXX_COMPILER=clang++
    -DCMAKE_VERBOSE_MAKEFILE=ON
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)
TEST=false
BENCHMARK=false

enable_tests() {
    CMAKE_ARGS+=(-DBUILD_TESTS=ON)
    TEST=true
}

enable_benchmarks() {
    CMAKE_ARGS+=(-DBUILD_BENCHMARKS=ON)
    BENCHMARK=true
}


run_tests() {
    if [ $TEST = true ]; then
        pushd ${BUILD_DIR}/src/tests
        ctest -j8
        popd
    fi
}

run_benchmarks() {
    CPUNAME=$(sed "s/[^a-zA-Z0-9]/ /g" <<< `uname -p` | xargs | sed "s/ /_/g")

    if [ $BENCHMARK = true ]; then
        pushd ${BUILD_DIR}/src/benchmarks
        logdir=${SRC_DIR}/bm_results/${CPUNAME}
        mkdir -p ${logdir}
        for bm in bm_*; do
            log=${logdir}/${bm:3}.csv
            ./${bm} ${log}
        done
        popd
    fi
}

for key in $@; do
    case $key in
        -t)
            enable_tests
            ;;
        -b)
            enable_benchmarks
            ;;
        -e)
            BUILD_DIR=`mktemp -d`
            ;;
        -a)
            enable_tests
            enable_benchmarks
            ;;
        -h)
            showhelp
            exit 0
            ;;
        *)
            echo "Unknown option $key"
            showhelp
            exit 1
            ;;
    esac
done

mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}
cmake ${CMAKE_ARGS[*]} ${SRC_DIR}
make -j8
run_tests
run_benchmarks
popd
