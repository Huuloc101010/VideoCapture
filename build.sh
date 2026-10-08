#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build()
{
    cmake -S ${SCRIPT_DIR} -B ${SCRIPT_DIR}/tmp \
    -G "Unix Makefiles" \
    -DCMAKE_CXX_COMPILER=g++-13

    cmake --build ${SCRIPT_DIR}/tmp -j$(nproc)
}

rebuild()
{
    clean
    build
}

clean()
{
    echo 'Cleaning. Deleting tmp directory'
    rm -rf ${SCRIPT_DIR}/tmp
}

if [ "$1" = "build" ]
then
    build
elif [ "$1" = "rebuild" ]
then
    rebuild
elif [ "$1" = "clean" ]
then
    clean
else
    echo "Unknown command"
fi