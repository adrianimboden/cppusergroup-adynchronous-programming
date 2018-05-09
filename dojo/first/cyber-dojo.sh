#!/bin/bash -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

mkdir -p ${HOME}/build
cd ${HOME}/build
bash -c "cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 $(cat /prebuilt/inst/debug/cmake_flags) ${DIR}"
cmake --build .

#for file in $(find ${DIR}/*/ -iname *.cpp); do
#	clang-tidy ${file} -p ${HOME}/build -quiet 2>/dev/null | grep -v "warnings generated." &
#done
#wait

./app_test
