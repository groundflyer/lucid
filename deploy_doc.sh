#!/bin/bash

showhelp() {
    echo "Usage: $0 [-h] [-d]"
    echo -e "\t-d\tbuild docs but don't deploy"
    echo -e "\t-h\tshow this help message and exit"
}

DO_DEPLOY=true

for key in $@; do
    case $key in
        -d)
            DO_DEPLOY=false
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

SRC_DIR=`realpath $(dirname "${BASH_SOURCE[0]}")`
BUILD_DIR=`mktemp -d`
DEPLOY_DIR=${1:-`mktemp -du`}

pushd ${BUILD_DIR}
cmake -DBUILD_DOC=ON ${SRC_DIR}
make doc
popd

if [ $DO_DEPLOY = false ]; then
    echo "Docs are in ${BUILD_DIR}/html"
    exit 0
fi

if [ ! -d ${DEPLOY_DIR} ]; then
    git clone git@github.com:groundflyer/lucid.git --branch=gh-pages --single-branch ${DEPLOY_DIR}
fi

rm -r ${DEPLOY_DIR}/*
cp -r ${BUILD_DIR}/html/* ${DEPLOY_DIR}
pushd ${DEPLOY_DIR}
git add -A
git commit -m "[auto] updating the docs"
git push origin gh-pages
popd
