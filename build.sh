#!/bin/bash
build()
{
    cmake -S . -B tmp \
    -G "Unix Makefiles" \
    -DCMAKE_CXX_COMPILER=g++-13

    cmake --build tmp -j$(nproc)
}

rebuild()
{
    clean
    build
}

clean()
{
    echo 'Cleaning. Deleting tmp directory'
    rm -rf tmp
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