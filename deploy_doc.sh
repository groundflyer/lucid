#!/bin/bash

SRC_DIR=`realpath $(dirname "${BASH_SOURCE[0]}")`
BUILD_DIR=`mktemp -d`
DEPLOY_DIR=${1:-`mktemp -du`}

pushd ${BUILD_DIR}
cmake -DBUILD_DOC=ON ${SRC_DIR}
make doc
popd

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
