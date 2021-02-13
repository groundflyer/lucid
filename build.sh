#!/bin/bash

showhelp() {
    echo "Usage: $0 [-h] [-t] [-b] [-d] [-e] [-a] [-c] [-clang]"
    echo -e "\t-t\tbuild and run tests"
    echo -e "\t-b\tbuild and run benchmarks"
    echo -e "\t-d\tbuild documentation"
    echo -e "\t-e\tuse temporary directory"
    echo -e "\t-a\tbuild all, then run benchmarks and tests"
    echo -e "\t-c\trun cmake only, don't build"
    echo -e "\t-clang\tuse clang"
    echo -e "\t-h\tshow this help message and exit"
}

SRC_DIR=`realpath $(dirname "${BASH_SOURCE[0]}")`
BUILD_DIR=${SRC_DIR}/build
CMAKE_ARGS=(
    -DCMAKE_VERBOSE_MAKEFILE=ON
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)
TEST=false
BENCHMARK=false
[ -z "${JOBS:-}" ] && JOBS=8
DO_BUILD=true
CXX=g++
TARGETS=()
[ -z "${BM_RUNS:-}" ] && BM_RUNS=4

enable_tests() {
    CMAKE_ARGS+=(-DBUILD_TESTS=ON)
    TEST=true
    TARGETS+=(tests)
}

enable_benchmarks() {
    CMAKE_ARGS+=(-DBUILD_BENCHMARKS=ON)
    BENCHMARK=true
    TARGETS+=(benchmarks)
}

enable_docs() {
    CMAKE_ARGS+=(-DBUILD_DOC=ON)
    TARGETS+=(doc)
}

use_clang() {
    CXX=clang++
    CMAKE_ARGS+=(-DCMAKE_CXX_COMPILER=${CXX})
}

run_tests() {
    if [ $TEST = true ]; then
        pushd ${BUILD_DIR}/src/tests
        ctest -j${JOBS}
        popd
    fi
}

run_benchmarks() {
    CPUNAME=$(sed "s/[^a-zA-Z0-9]/ /g" <<< `uname -p` | xargs | sed "s/ /_/g")
    COMPILER=${CXX}-`${CXX} --version | head -n1 | cut -f3 -d' '`

    if [ $BENCHMARK = true ]; then
        pushd ${BUILD_DIR}/src/benchmarks
        logdir=${SRC_DIR}/bm_results/${COMPILER}/${CPUNAME}
        mkdir -p ${logdir}
        for bm in bm_*; do
            log=${logdir}/${bm:3}.csv
            for i in `seq 1 ${BM_RUNS}`; do
                echo "Running ${bm} ${i}/${BM_RUNS}"
                ./${bm} -l ${log}
            done
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
        -d)
            enable_docs
            ;;
        -e)
            BUILD_DIR=`mktemp -d`
            ;;
        -a)
            enable_tests
            enable_benchmarks
            enable_docs
            TARGETS=(all)
            ;;
        -c)
            DO_BUILD=false
            ;;
        -clang)
            use_clang
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

if [ $CXX = "clang++" ]; then
    BUILD_DIR+="_clang"
else
    BUILD_DIR+="_gcc"
fi

mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}
cmake ${CMAKE_ARGS[*]} ${SRC_DIR}
if [ $DO_BUILD = true ]; then
    make -j${JOBS} ${TARGETS[*]}
    run_tests
    run_benchmarks
fi
popd
