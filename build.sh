#!/bin/bash

showhelp() {
    echo "Usage: $0 [-t] [-b] [-d] [-e] [-a] [-c]"
    echo -e "\t-t\tbuild and run tests"
    echo -e "\t-b\tbuild and run benchmarks"
    echo -e "\t-d\tbuild documentation"
    echo -e "\t-e\tuse temporary directory"
    echo -e "\t-a\tbuild all, then run benchmarks and tests"
    echo -e "\t-c\trun cmake only, don't build"
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
[ -z "${JOBS:-}" ] && JOBS=8
DO_BUILD=true

enable_tests() {
    CMAKE_ARGS+=(-DBUILD_TESTS=ON)
    TEST=true
}

enable_benchmarks() {
    CMAKE_ARGS+=(-DBUILD_BENCHMARKS=ON)
    BENCHMARK=true
}

enable_docs() {
    CMAKE_ARGS+=(-DBUILD_DOC=ON)
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
            ;;
        -c)
            DO_BUILD=false
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
if [ $DO_BUILD = true ]; then
    make -j${JOBS}
    run_tests
    run_benchmarks
fi
popd
